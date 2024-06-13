#ifndef _SCTE35_H_
#define _SCTE35_H_

#include "config.h"
#include "cmdutils.h"

#define SPLICE_INFO_FIXED_SIZE 14
#define MAX_SCTE35_SPLICE_COMPONENTS 10

// values for types of SCTE35 splice commands
enum {
    SCTE35_CMD_NULL = 0,
    SCTE35_CMD_SCHEDULE = 4,
    SCTE35_CMD_INSERT = 5,
    SCTE35_CMD_TIME_SIGNAL = 6,
    SCTE35_CMD_BW_RESERVATION = 7,
    SCTE35_CMD_PRIVATE_CMD = 0xff
};

// specifies duration of commerical break
// if auto_return is one, then splicing device uses duration to determine when to return to network feed from a break
// if auto_return is zero, a splice_insert will end a break
typedef struct SCTE35BreakDuration {
    int auto_return; 
    int reserved; 
    uint64_t duration; // in terms of program's ninety kHz clock
} SCTE35BreakDuration;

// specifies time of splice event
typedef struct SCTE35SpliceTime {
    int time_specified_flag; // if one, indicates pts_time is set
    int reserved;
    uint64_t pts_time; // in terms of program's ninety kHz clock
    int byte_offset;
} SCTE35SpliceTime;

typedef struct SCTE35SpliceComponent {
    int component_tag; // represents elementary PID stream containing splice point
    SCTE35SpliceTime time;
} SCTE35SpliceComponent;

typedef struct SCTE35SpliceInsert {
    int splice_event_id;
    int splice_event_cancel_indicator; // if set to one, means previous sent splice event was canceled
    int reserved_1;
    int out_of_network_indicator; // if one, means exiting network feed. otherwise, means returning
    int program_splice_flag; // indicates whether in program splice or component splice mode
    int duration_flag; // if one, break_duration field exists
    int splice_immediate_flag;
    int reserved_2;
    SCTE35SpliceTime time;
    int component_count;
    //SCTE35SpliceComponent component_info[MAX_SCTE35_SPLICE_COMPONENTS];
    SCTE35BreakDuration break_duration;
    int unique_program_id;
    int avail_num;
    int avails_expected;
} SCTE35SpliceInsert;

typedef struct SCTE35SpliceSchedule {
    int dummy;
} SCTE35SpliceSchedule;

typedef struct SCTE35Time {
    int dummy;
} SCTE35Time;

typedef struct SCTE35SpliceDesc {
    int dummy;
} SCTE35SpliceDesc;

// overall struct to hold parsed contents of SCTE35 packet
typedef struct SCTE35ParseSection {
    int table_id;
    int section_syntax_indicator;
    int private_indicator;
    int reserved;
    int section_length;
    int protocol_version;
    int encrypted_packet;
    int encryption_algorithm;
    uint64_t pts_adjustment;
    int cw_index;
    int tier;
    int splice_command_length;
    int splice_command_type;

    union cmd {
        SCTE35SpliceInsert insert;
        SCTE35SpliceSchedule schedule;
        SCTE35Time time_signal; 
    } cmd;

    int descriptor_loop_length;
    //SCTE35SpliceDesc splice_descriptor;
    int num_alignment_bytes;
    uint32_t e_crc;
    uint32_t crc;
} SCTE35ParseSection;

void scte35_parse_init(SCTE35ParseSection *scte35_ptr);
int parse_insert(unsigned char *cmd, unsigned char *table, SCTE35ParseSection *scte35_ptr);
/*int parse_splice_descriptor(unsigned char *field, unsigned char *table, SCTE35SpliceDescriptor *splice_descriptor);*/

#endif
