/****************************************************************
 *								*
 *	Copyright 2001, 2008 Fidelity Information Services, Inc	*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#include "mdef.h"

#include "gtm_string.h"
#include "gtm_time.h"
#include <sys/mman.h>
#include <errno.h>
#include "gtm_unistd.h"
#include <signal.h>
#include "gtm_statvfs.h"	/* for GTM_BAVAIL_TYPE */

#include "buddy_list.h"
#include "gdsroot.h"
#include "gdskill.h"
#include "gtm_facility.h"
#include "fileinfo.h"
#include "gdsbml.h"
#include "gdsbt.h"
#include "gdsfhead.h"
#include "gdscc.h"
#include "filestruct.h"
#include "gtmio.h"
#include "iosp.h"
#include "jnl.h"
#include "hashtab_int4.h"     /* needed for tp.h */
#include "tp.h"
#include "eintr_wrappers.h"
#include "send_msg.h"
#include "gt_timer.h"
#include "mmseg.h"
#include "gdsblk.h"		/* needed for gds_blk_downgrade.h */
#include "gds_blk_downgrade.h"	/* for IS_GDS_BLK_DOWNGRADE_NEEDED macro */

/* Include prototypes */
#include "bit_set.h"
#include "disk_block_available.h"
#include "gds_map_moved.h"
#include "gtmmsg.h"
#include "gdsfilext.h"
#include "bm_getfree.h"

#define	      GDSFILEXT_CLNUP { if (need_to_restore_mask)				\
					sigprocmask(SIG_SETMASK, &savemask, NULL);	\
				if (!was_crit)						\
					rel_crit(gv_cur_region);			\
				cs_addrs->extending = FALSE;				\
			}

GBLREF	sigset_t	blockalrm;
GBLREF	sgmnt_addrs	*cs_addrs;
GBLREF	sgmnt_data_ptr_t cs_data;
GBLREF	unsigned char	cw_set_depth;
GBLREF	short		dollar_tlevel;
GBLREF	gd_region	*gv_cur_region;
GBLREF	inctn_opcode_t	inctn_opcode;
GBLREF	boolean_t	mu_reorg_process;
GBLREF	uint4		process_id;
GBLREF	boolean_t	run_time;
GBLREF	sgm_info	*sgm_info_ptr;
GBLREF	unsigned int	t_tries;
GBLREF	jnl_gbls_t	jgbl;
GBLREF	inctn_detail_t	inctn_detail;			/* holds detail to fill in to inctn jnl record */
GBLREF	boolean_t	gtm_dbfilext_syslog_disable;	/* control whether db file extension message is logged or not */

OS_PAGE_SIZE_DECLARE

#ifdef DEBUG_DB64
/* if debugging large address stuff, make all memory segments allocate above 4G line */
GBLREF	sm_uc_ptr_t	next_smseg;
#endif

uint4	 gdsfilext (uint4 blocks, uint4 filesize)
{
	sm_uc_ptr_t		old_base[2];
	boolean_t		was_crit, need_to_restore_mask = FALSE;
	char			*buff;
	int			mm_prot, result, save_errno, status;
	uint4			new_bit_maps, bplmap, map, new_blocks, new_total, max_tot_blks;
	uint4			jnl_status, to_wait, to_msg, wait_period;
	GTM_BAVAIL_TYPE		avail_blocks;
	sgmnt_data_ptr_t	tmp_csd;
	off_t			new_eof;
	trans_num		curr_tn;
	unix_db_info		*udi;
	sigset_t		savemask;
	inctn_opcode_t		save_inctn_opcode;
	int4			prev_extend_blks_to_upgrd;
	jnl_private_control	*jpc;
	jnl_buffer_ptr_t	jbp;

	error_def(ERR_DBFILERR);
	error_def(ERR_DBFILEXT);
	error_def(ERR_DSKSPACEFLOW);
	error_def(ERR_JNLFLUSH);
	error_def(ERR_TEXT);
	error_def(ERR_TOTALBLKMAX);
	error_def(ERR_WAITDSKSPACE);

#ifdef __hppa
	if (dba_mm == cs_addrs->hdr->acc_meth)
		return (uint4)(NO_FREE_SPACE); /* should this be changed to show extension not allowed ? */
#endif

	/* Both blocks and total blocks are unsigned ints so make sure we aren't asking for huge numbers that will
	   overflow and end up doing silly things.
	*/
	assert(blocks <= (MAXTOTALBLKS(cs_data) - cs_data->trans_hist.total_blks));

	if (!blocks)
		return (uint4)(NO_FREE_SPACE); /* should this be changed to show extension not enabled ? */
	bplmap = cs_data->bplmap;
	/* new total of non-bitmap blocks will be number of current, non-bitmap blocks, plus new blocks desired
	   There are (bplmap - 1) non-bitmap blocks per bitmap, so add (bplmap - 2) to number of non-bitmap blocks
		and divide by (bplmap - 1) to get total number of bitmaps for expanded database. (must round up in this
		manner as every non-bitmap block must have an associated bitmap)
	   Current number of bitmaps is (total number of current blocks + bplmap - 1) / bplmap.
	   Subtract current number of bitmaps from number needed for expanded database to get number of new bitmaps needed.
	*/
	new_bit_maps = DIVIDE_ROUND_UP(cs_data->trans_hist.total_blks
			- DIVIDE_ROUND_UP(cs_data->trans_hist.total_blks, bplmap) + blocks, bplmap - 1)
			- DIVIDE_ROUND_UP(cs_data->trans_hist.total_blks, bplmap);
	new_blocks = blocks + new_bit_maps;
	assert(0 < (int)new_blocks);
	udi = FILE_INFO(gv_cur_region);
	if (0 != (save_errno = disk_block_available(udi->fd, &avail_blocks, FALSE)))
	{
		send_msg(VARLSTCNT(5) ERR_DBFILERR, 2, DB_LEN_STR(gv_cur_region), save_errno);
		rts_error(VARLSTCNT(5) ERR_DBFILERR, 2, DB_LEN_STR(gv_cur_region), save_errno);
	} else
	{
		avail_blocks = avail_blocks / (cs_data->blk_size / DISK_BLOCK_SIZE);
		if ((blocks * EXTEND_WARNING_FACTOR) > avail_blocks)
		{
			send_msg(VARLSTCNT(5) ERR_DSKSPACEFLOW, 3, DB_LEN_STR(gv_cur_region),
					(uint4)(avail_blocks - ((new_blocks <= avail_blocks) ? new_blocks : 0)));
#ifndef __MVS__
			if (blocks > (uint4)avail_blocks)
				return (uint4)(NO_FREE_SPACE);
#endif
		}
	}
	cs_addrs->extending = TRUE;
	was_crit = cs_addrs->now_crit;
	/* If we are coming from mupip_extend (which gets crit itself) we better have waited for any unfreezes to occur */
	assert(!was_crit || CDB_STAGNATE == t_tries || FALSE == cs_data->freeze);
	for ( ; ; )
	{	/* If we are in the final retry and already hold crit, it is possible that csd->wc_blocked is also set to TRUE
		 * (by a concurrent process in phase2 which encountered an error in the midst of commit and secshr_db_clnup
		 * finished the job for it). In this case we do NOT want to invoke wcs_recover as that will update the "bt"
		 * transaction numbers without correspondingly updating the history transaction numbers (effectively causing
		 * a cdb_sc_blkmod type of restart). Therefore do NOT call grab_crit (which unconditionally invokes wcs_recover)
		 * if we already hold crit.
		 */
		if (!was_crit)
			grab_crit(gv_cur_region);
		if (FALSE == cs_data->freeze)
			break;
		rel_crit(gv_cur_region);
		if (was_crit)
		{	/* Two cases.
			 * (i)  Final retry and in TP. We might be holding crit in other regions too.
			 *	We can't do a grab_crit() on this region again unless it is deadlock-safe.
			 *      To be on the safer side, we do a restart. The tp_restart() logic will wait
			 *	for this region's freeze to be removed before grabbing crit.
			 * (ii) Final retry and not in TP. In that case too, it is better to restart in case there is
			 *	some validation code that shortcuts the checking for the final retry assuming we were
			 *	in crit from t_begin() to t_end(). t_retry() has logic that will wait for unfreeze.
			 * In either case, we need to restart. Returning EXTEND_UNFREEZECRIT will cause one in t_end/tp_tend.
			 */
			return (uint4)(EXTEND_UNFREEZECRIT);
		}
		while (cs_data->freeze)
			hiber_start(1000);
	}
	assert(cs_addrs->ti->total_blks == cs_data->trans_hist.total_blks);
	if (cs_data->trans_hist.total_blks != filesize)
	{
		/* somebody else has already extended it, since we are in crit, this is trust-worthy
		 * however, in case of MM, we still need to remap the database */
		assert(cs_data->trans_hist.total_blks > filesize);
		GDSFILEXT_CLNUP;
		return (SS_NORMAL);
	}
	if (run_time && (2 * ((0 < dollar_tlevel) ? sgm_info_ptr->cw_set_depth : cw_set_depth) < cs_addrs->ti->free_blocks))
	{
		if (FALSE == was_crit)
		{
			rel_crit(gv_cur_region);
			return (uint4)(EXTEND_SUSPECT);
		}
		/* If free_blocks counter is not ok, then correct it. Do the check again. If still fails, then GTMASSERT. */
		if (is_free_blks_ctr_ok() ||
				(2 * ((0 < dollar_tlevel) ? sgm_info_ptr->cw_set_depth : cw_set_depth) < cs_addrs->ti->free_blocks))
			GTMASSERT;	/* held crit through bm_getfree into gdsfilext and still didn't get it right */
	}
	if (JNL_ENABLED(cs_data))
	{
		if (!jgbl.dont_reset_gbl_jrec_time)
			SET_GBL_JREC_TIME;	/* needed before jnl_ensure_open as that can write jnl records */
		jpc = cs_addrs->jnl;
		jbp = jpc->jnl_buff;
		/* Before writing to jnlfile, adjust jgbl.gbl_jrec_time if needed to maintain time order
		 * of jnl records. This needs to be done BEFORE the jnl_ensure_open as that could write
		 * journal records (if it decides to switch to a new journal file).
		 */
		ADJUST_GBL_JREC_TIME(jgbl, jbp);
		jnl_status = jnl_ensure_open();
		if (jnl_status)
		{
			GDSFILEXT_CLNUP;
			send_msg(VARLSTCNT(6) jnl_status, 4, JNL_LEN_STR(cs_data), DB_LEN_STR(gv_cur_region));
			return (uint4)(NO_FREE_SPACE);	/* should have better return status */
		}
	}
	if (dba_mm == cs_addrs->hdr->acc_meth)
	{
#if defined(UNTARGETED_MSYNC)
		status = msync((caddr_t)cs_addrs->db_addrs[0], (size_t)(cs_addrs->db_addrs[1] - cs_addrs->db_addrs[0]), MS_SYNC);
#else
		cs_addrs->nl->mm_extender_pid = process_id;
		status = wcs_wtstart(gv_cur_region, 0);
		cs_addrs->nl->mm_extender_pid = 0;
		if (0 != cs_addrs->acc_meth.mm.mmblk_state->mmblkq_active.fl)
			GTMASSERT;
		status = 0;
#endif
		if (0 == status)
		{
			/* Block SIGALRM for the duration when cs_data and cs_addrs are out of sync */
			sigprocmask(SIG_BLOCK, &blockalrm, &savemask);
			need_to_restore_mask = TRUE;
			tmp_csd = cs_data;
			cs_data = (sgmnt_data_ptr_t)malloc(sizeof(*cs_data));
			memcpy((sm_uc_ptr_t)cs_data, (uchar_ptr_t)tmp_csd, sizeof(*cs_data));
			status = munmap((caddr_t)cs_addrs->db_addrs[0],
					     (size_t)(cs_addrs->db_addrs[1] - cs_addrs->db_addrs[0]));
#ifdef DEBUG_DB64
			if (-1 != status)
				rel_mmseg((caddr_t)cs_addrs->db_addrs[0]);
#endif
		} else
			tmp_csd = NULL;
		if (0 != status)
		{
			if (tmp_csd)
			{
				free(cs_data);
				cs_data = tmp_csd;
			}
			GDSFILEXT_CLNUP;
			send_msg(VARLSTCNT(5) ERR_DBFILERR, 2, DB_LEN_STR(gv_cur_region), status);
			return (uint4)(NO_FREE_SPACE);
		}
		cs_addrs->hdr = cs_data;
		cs_addrs->ti = &cs_data->trans_hist;
	}
	if (new_blocks + cs_data->trans_hist.total_blks > MAXTOTALBLKS(cs_data))
	{
		GDSFILEXT_CLNUP;
		send_msg(VARLSTCNT(1) ERR_TOTALBLKMAX);
		return (uint4)(NO_FREE_SPACE);
	}
	CHECK_TN(cs_addrs, cs_data, cs_data->trans_hist.curr_tn);	/* can issue rts_error TNTOOLARGE */
	assert(0 < (int)new_blocks);
	new_total = cs_data->trans_hist.total_blks + new_blocks;
	new_eof = ((off_t)(cs_data->start_vbn - 1) * DISK_BLOCK_SIZE) + ((off_t)new_total * cs_data->blk_size);
	buff = (char *)malloc(DISK_BLOCK_SIZE);
	memset(buff, 0, DISK_BLOCK_SIZE);
	LSEEKWRITE(udi->fd, new_eof, buff, DISK_BLOCK_SIZE, save_errno);
	if ((ENOSPC == save_errno) && run_time)
	{
		/* try to write it every second, and send message to operator
		 * log every 1/20 of cs_data->wait_disk_space
		 */
		wait_period = to_wait = DIVIDE_ROUND_UP(cs_data->wait_disk_space, CDB_STAGNATE + 1);
		to_msg = (to_wait / 8) ? (to_wait / 8) : 1;		/* send around 8 messages during 1 wait_period */
		while ((to_wait > 0) && (ENOSPC == save_errno))
		{
			if ((to_wait == cs_data->wait_disk_space) || (to_wait % to_msg == 0))
			{
				send_msg(VARLSTCNT(11) ERR_WAITDSKSPACE, 4, process_id,
					to_wait + (CDB_STAGNATE - t_tries) * wait_period, DB_LEN_STR(gv_cur_region),
					ERR_TEXT, 2,
					RTS_ERROR_TEXT("Please make more disk space available or shutdown GT.M to avoid data loss"),
					save_errno);
				gtm_putmsg(VARLSTCNT(11) ERR_WAITDSKSPACE, 4, process_id,
					to_wait + (CDB_STAGNATE - t_tries) * wait_period, DB_LEN_STR(gv_cur_region),
					ERR_TEXT, 2,
					RTS_ERROR_TEXT("Please make more disk space available or shutdown GT.M to avoid data loss"),
					save_errno);
			}
			if (!was_crit)
				rel_crit(gv_cur_region);
			hiber_start(1000);
			to_wait--;
			if (!was_crit)
				grab_crit(gv_cur_region);
			LSEEKWRITE(udi->fd, new_eof, buff, DISK_BLOCK_SIZE, save_errno);
		}
	}
	free(buff);
	if (0 != save_errno)
	{
		GDSFILEXT_CLNUP;
		if (ENOSPC == save_errno)
			return (uint4)(NO_FREE_SPACE);
		send_msg(VARLSTCNT(5) ERR_DBFILERR, 2, DB_LEN_STR(gv_cur_region), save_errno);
		return (uint4)(NO_FREE_SPACE);
	}
	DEBUG_ONLY(prev_extend_blks_to_upgrd = cs_data->blks_to_upgrd;)
	/* inctn_detail.blks_to_upgrd_delta holds the increase in "csd->blks_to_upgrd" due to the file extension */
	inctn_detail.blks_to_upgrd_delta = (IS_GDS_BLK_DOWNGRADE_NEEDED(cs_data->desired_db_format) ? new_bit_maps : 0);
	if (JNL_ENABLED(cs_data))
	{
		save_inctn_opcode = inctn_opcode;
		if (mu_reorg_process)
			inctn_opcode = inctn_gdsfilext_mu_reorg;
		else
			inctn_opcode = inctn_gdsfilext_gtm;
		if (0 == jpc->pini_addr)
			jnl_put_jrt_pini(cs_addrs);
		jnl_write_inctn_rec(cs_addrs);
		inctn_opcode = save_inctn_opcode;
		/* Harden INCTN to disk before updating/flushing database. This will ensure that any positive adjustment to the
		 * blks_to_upgrd counter (stored in the inctn record) is seen by recovery before a V4 format bitmap block is.
		 */
		jnl_status = jnl_flush(gv_cur_region);
		if (SS_NORMAL != jnl_status)
		{
			send_msg(VARLSTCNT(9) ERR_JNLFLUSH, 2, JNL_LEN_STR(cs_data),
				ERR_TEXT, 2, RTS_ERROR_TEXT("Error with journal flush during gdsfilext"),
				jnl_status);
			assert(NOJNL == jpc->channel); /* jnl file lost has been triggered */
			/* In this routine, all code that follows from here on does not assume anything about the
			 * journaling characteristics of this database so it is safe to continue execution even though
			 * journaling got closed in the middle. Let the caller deal with this situation.
			 */
		}
	}
	if (new_bit_maps)
	{
		for (map = ROUND_UP(cs_data->trans_hist.total_blks, bplmap); map < new_total; map += bplmap)
		{
			DEBUG_ONLY(new_bit_maps--;)
			if (SS_NORMAL != (status = bml_init(map)))
			{
				GDSFILEXT_CLNUP;
				send_msg(VARLSTCNT(5) ERR_DBFILERR, 2, DB_LEN_STR(gv_cur_region), status);
				return (uint4)(NO_FREE_SPACE);
			}
		}
		assert(0 == new_bit_maps);
	}
	assert(cs_data->blks_to_upgrd == (inctn_detail.blks_to_upgrd_delta + prev_extend_blks_to_upgrd));
	if (dba_mm == cs_addrs->hdr->acc_meth)
	{	/* On 32 bit aix, is it possible we can have now increased the file size past what we can map ? */
		mm_prot = cs_addrs->read_write ? (PROT_READ | PROT_WRITE) : PROT_READ;
		old_base[0] = cs_addrs->db_addrs[0];
		old_base[1] = cs_addrs->db_addrs[1];
#ifdef DEBUG_DB64
		if (-1 == ((sm_long_t)(cs_addrs->db_addrs[0] = (sm_uc_ptr_t)mmap((caddr_t)get_mmseg((size_t)new_eof),
										(size_t)new_eof, mm_prot,
										GTM_MM_FLAGS, udi->fd, (off_t)0))))
		{
			GDSFILEXT_CLNUP;
			send_msg(VARLSTCNT(5) ERR_DBFILERR, 2, DB_LEN_STR(gv_cur_region), errno);
			return (uint4)(NO_FREE_SPACE);
		}
		put_mmseg((caddr_t)(cs_addrs->db_addrs[0]), (size_t)new_eof);
#else
		if (-1 == ((sm_long_t)(cs_addrs->db_addrs[0] = (sm_uc_ptr_t)mmap((caddr_t)NULL, (size_t)new_eof, mm_prot,
										GTM_MM_FLAGS, udi->fd, (off_t)0))))
		{
			GDSFILEXT_CLNUP;
			send_msg(VARLSTCNT(5) ERR_DBFILERR, 2, DB_LEN_STR(gv_cur_region), errno);
			return (uint4)(NO_FREE_SPACE);
		}
#endif
		free(cs_data);			/* note current assumption that cs_data has not changed since memcpy above */
		/* In addition to updating the internal map values, gds_map_moved sets cs_data to point to the remapped file */
		gds_map_moved(cs_addrs->db_addrs[0], old_base[0], old_base[1], new_eof);
                cs_addrs->total_blks = new_total;       /* Local copy to test if file has extended */
 	}
	assert(0 < (int)blocks);
	assert(0 < (int)(cs_addrs->ti->free_blocks + blocks));
	cs_addrs->ti->free_blocks += blocks;
	blocks = cs_data->trans_hist.total_blks;
	cs_addrs->ti->total_blks = new_total;
	if (blocks / bplmap * bplmap != blocks)
	{
		bit_set(blocks / bplmap, MM_ADDR(cs_data)); /* Mark old last local map as having space */
		if ((int4)blocks > cs_addrs->nl->highest_lbm_blk_changed)
			cs_addrs->nl->highest_lbm_blk_changed = blocks;
	}
	curr_tn = cs_addrs->ti->curr_tn;
	assert(cs_addrs->ti->early_tn == cs_addrs->ti->curr_tn);
	/* do not increment transaction number for forward recovery */
	if (!jgbl.forw_phase_recovery || JNL_ENABLED(cs_data))
	{
		cs_data->trans_hist.early_tn = cs_data->trans_hist.curr_tn + 1;
		INCREMENT_CURR_TN(cs_data);
	}
	fileheader_sync(gv_cur_region);
	GDSFILEXT_CLNUP;
	if (!gtm_dbfilext_syslog_disable)
		send_msg(VARLSTCNT(7) ERR_DBFILEXT, 5, DB_LEN_STR(gv_cur_region), blocks, new_total, &curr_tn);
	return (SS_NORMAL);
}
