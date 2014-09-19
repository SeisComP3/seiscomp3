/*
 * ./src/scsireg.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: scsireg.h,v 1.1 2003/05/16 10:40:28 root Exp $
 */

/*
 * $Log: scsireg.h,v $
 * Revision 1.1  2003/05/16 10:40:28  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/04/01 18:00:18  root
 * #
 *
 * Revision 1.2  2003/04/01 17:54:54  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:16  root
 * #
 *
 */

#ifndef __SCSIREG_H__
#define __SCSIREG_H__


/* OPCODES */

#define SCSI_OP_TEST_UNIT_READY       0x00
#define SCSI_OP_REZERO_UNIT           0x01
#define SCSI_OP_REQUEST_SENSE         0x03
#define SCSI_OP_FORMAT_UNIT           0x04
#define SCSI_OP_READ_BLOCK_LIMITS     0x05
#define SCSI_OP_REASSIGN_BLOCKS       0x07
#define SCSI_OP_READ_6                0x08
#define SCSI_OP_WRITE_6               0x0a
#define SCSI_OP_SEEK_6                0x0b
#define SCSI_OP_READ_REVERSE          0x0f
#define SCSI_OP_WRITE_FILEMARKS       0x10
#define SCSI_OP_SPACE                 0x11
#define SCSI_OP_INQUIRY               0x12
#define SCSI_OP_RECOVER_BUFFERED_DATA 0x14
#define SCSI_OP_MODE_SELECT           0x15
#define SCSI_OP_RESERVE               0x16
#define SCSI_OP_RELEASE               0x17
#define SCSI_OP_COPY                  0x18
#define SCSI_OP_ERASE                 0x19
#define SCSI_OP_MODE_SENSE            0x1a
#define SCSI_OP_START_STOP            0x1b
#define SCSI_OP_RECEIVE_DIAGNOSTIC    0x1c
#define SCSI_OP_SEND_DIAGNOSTIC       0x1d
#define SCSI_OP_ALLOW_MEDIUM_REMOVAL  0x1e

#define SCSI_OP_SET_WINDOW            0x24
#define SCSI_OP_READ_CAPACITY         0x25
#define SCSI_OP_READ_10               0x28
#define SCSI_OP_WRITE_10              0x2a
#define SCSI_OP_SEEK_10               0x2b
#define SCSI_OP_WRITE_VERIFY          0x2e
#define SCSI_OP_VERIFY                0x2f
#define SCSI_OP_SEARCH_HIGH           0x30
#define SCSI_OP_SEARCH_EQUAL          0x31
#define SCSI_OP_SEARCH_LOW            0x32
#define SCSI_OP_SET_LIMITS            0x33
#define SCSI_OP_PRE_FETCH             0x34
#define SCSI_OP_READ_POSITION         0x34
#define SCSI_OP_SYNCHRONIZE_CACHE     0x35
#define SCSI_OP_LOCK_UNLOCK_CACHE     0x36
#define SCSI_OP_READ_DEFECT_DATA      0x37
#define SCSI_OP_MEDIUM_SCAN           0x38
#define SCSI_OP_COMPARE               0x39
#define SCSI_OP_COPY_VERIFY           0x3a
#define SCSI_OP_WRITE_BUFFER          0x3b
#define SCSI_OP_READ_BUFFER           0x3c
#define SCSI_OP_UPDATE_BLOCK          0x3d
#define SCSI_OP_READ_LONG             0x3e
#define SCSI_OP_WRITE_LONG            0x3f
#define SCSI_OP_CHANGE_DEFINITION     0x40
#define SCSI_OP_WRITE_SAME            0x41
#define SCSI_OP_READ_TOC              0x43
#define SCSI_OP_LOG_SELECT            0x4c
#define SCSI_OP_LOG_SENSE             0x4d
#define SCSI_OP_MODE_SELECT_10        0x55
#define SCSI_OP_RESERVE_10            0x56
#define SCSI_OP_RELEASE_10            0x57
#define SCSI_OP_MODE_SENSE_10         0x5a
#define SCSI_OP_PERSISTENT_RESERVE_IN 0x5e
#define SCSI_OP_PERSISTENT_RESERVE_OUT 0x5f
#define SCSI_OP_MOVE_MEDIUM           0xa5
#define SCSI_OP_READ_12               0xa8
#define SCSI_OP_WRITE_12              0xaa
#define SCSI_OP_WRITE_VERIFY_12       0xae
#define SCSI_OP_SEARCH_HIGH_12        0xb0
#define SCSI_OP_SEARCH_EQUAL_12       0xb1
#define SCSI_OP_SEARCH_LOW_12         0xb2
#define SCSI_OP_READ_ELEMENT_STATUS   0xb8
#define SCSI_OP_SEND_VOLUME_TAG       0xb6
#define SCSI_OP_WRITE_LONG_2          0xea

/*  STATUS CODES */

#define SCSI_ST_GOOD                 0x00
#define SCSI_ST_CHECK_CONDITION      0x01
#define SCSI_ST_CONDITION_GOOD       0x02
#define SCSI_ST_BUSY                 0x04
#define SCSI_ST_INTERMEDIATE_GOOD    0x08
#define SCSI_ST_INTERMEDIATE_C_GOOD  0x0a
#define SCSI_ST_RESERVATION_CONFLICT 0x0c
#define SCSI_ST_COMMAND_TERMINATED   0x11
#define SCSI_ST_QUEUE_FULL           0x14

#define SCSI_ST_STATUS_MASK          0x3e

/* SENSE KEYS */

#define SCSI_SK_NO_SENSE            0x00
#define SCSI_SK_RECOVERED_ERROR     0x01
#define SCSI_SK_NOT_READY           0x02
#define SCSI_SK_MEDIUM_ERROR        0x03
#define SCSI_SK_HARDWARE_ERROR      0x04
#define SCSI_SK_ILLEGAL_REQUEST     0x05
#define SCSI_SK_UNIT_ATTENTION      0x06
#define SCSI_SK_DATA_PROTECT        0x07
#define SCSI_SK_BLANK_CHECK         0x08
#define SCSI_SK_COPY_ABORTED        0x0a
#define SCSI_SK_ABORTED_COMMAND     0x0b
#define SCSI_SK_VOLUME_OVERFLOW     0x0d
#define SCSI_SK_MISCOMPARE          0x0e


 /*  DEVICE TYPES */

#define SCSI_TYPE_DISK           0x00
#define SCSI_TYPE_TAPE           0x01
#define SCSI_TYPE_PRINTER        0x02
#define SCSI_TYPE_PROCESSOR      0x03 /* HP scanners use this */
#define SCSI_TYPE_WORM           0x04 /* Treated as ROM by our system */
#define SCSI_TYPE_ROM            0x05
#define SCSI_TYPE_SCANNER        0x06
#define SCSI_TYPE_MOD            0x07 /* Magneto-optical disk - 
                                         * - treated as TYPE_DISK */
#define SCSI_TYPE_MEDIUM_CHANGER 0x08
#define SCSI_TYPE_COMM           0x09 /* Communications device */
#define SCSI_TYPE_ENCLOSURE      0x0d /* Enclosure Services Device */
#define SCSI_TYPE_NO_LUN         0x7f

/* MESAGES */

#define SCSI_MSG_COMMAND_COMPLETE    0x00
#define SCSI_MSG_EXTENDED_MESSAGE    0x01
#define SCSI_MSG_EXTENDED_MODIFY_DATA_POINTER    0x00
#define SCSI_MSG_EXTENDED_SDTR                   0x01
#define SCSI_MSG_EXTENDED_EXTENDED_IDENTIFY      0x02 /* SCSI-I only */
#define SCSI_MSG_EXTENDED_WDTR                   0x03
#define SCSI_MSG_SAVE_POINTERS       0x02
#define SCSI_MSG_RESTORE_POINTERS    0x03
#define SCSI_MSG_DISCONNECT          0x04
#define SCSI_MSG_INITIATOR_ERROR     0x05
#define SCSI_MSG_ABORT               0x06
#define SCSI_MSG_MESSAGE_REJECT      0x07
#define SCSI_MSG_NOP                 0x08
#define SCSI_MSG_MSG_PARITY_ERROR    0x09
#define SCSI_MSG_LINKED_CMD_COMPLETE 0x0a
#define SCSI_MSG_LINKED_FLG_CMD_COMPLETE 0x0b
#define SCSI_MSG_BUS_DEVICE_RESET    0x0c

#define SCSI_MSG_INITIATE_RECOVERY   0x0f /* SCSI-II only */
#define SCSI_MSG_RELEASE_RECOVERY    0x10 /* SCSI-II only */

#define SCSI_MSG_SIMPLE_QUEUE_TAG    0x20
#define SCSI_MSG_HEAD_OF_QUEUE_TAG   0x21
#define SCSI_MSG_ORDERED_QUEUE_TAG   0x22

#endif /* __SCSIREG_H__ */
