#ifndef _SCTE35_H_
#define _SCTE35_H_

#include "config.h"
#include "cmdutils.h"

#define SPLICE_INFO_FIXED_SIZE 14
#define MAX_SCTE35_SPLICE_COMPONENTS 10
#define SPLICE_DESCRIPTOR_DATA_MAX_SIZE 256

// values for types of errors in scte35.c
enum {
    SCTE35_MP_ERR_UNSUPPORTED_CMD = -1,
    SCTE35_MP_ERR_TABLE_ID = -2
};

// values for specific  SCTE35 splice commands
enum {
    SCTE35_CMD_NULL = 0,
    SCTE35_CMD_SCHEDULE = 4,
    SCTE35_CMD_INSERT = 5,
    SCTE35_CMD_TIME_SIGNAL = 6,
    SCTE35_CMD_BW_RESERVATION = 7,
    SCTE35_CMD_PRIVATE_CMD = 0xff
};

// values for specific SCTE35 splice descriptors
enum {
    SCTE35_SPLICE_DESCRIPTOR_AVAIL = 0,
    SCTE35_SPLICE_DESCRIPTOR_DTMF = 1,
    SCTE35_SPLICE_DESCRIPTOR_SEGMENTATION = 2,
    SCTE35_SPLICE_DESCRIPTOR_TIME = 3
};

// if auto_return is one, then splicing device uses duration to determine when to return to network feed from a break
// if auto_return is zero, a splice_insert will end a break
typedef struct SCTE35BreakDuration {
    int auto_return; 
    int reserved; 
    uint64_t duration; // in terms of program's ninety kHz clock
} SCTE35BreakDuration;

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

// sent at least once per splice event
typedef struct SCTE35SpliceInsert {
    int splice_event_id;
    int splice_event_cancel_indicator; // if set to one, means previous sent splice event was canceled
    int reserved_1;
    int out_of_network_indicator; // if one, means exiting network feed. otherwise, means returning
    int program_splice_flag; // indicates whether in program splice or component splice mode
    int duration_flag; // if one, break_duration field exists
    int splice_immediate_flag; // if one, indicates no SCTE35SpliceTime field
    int reserved_2;
    SCTE35SpliceTime time;
    int component_count; // number of elementary PID streams
    SCTE35SpliceComponent component_info[MAX_SCTE35_SPLICE_COMPONENTS];
    SCTE35BreakDuration break_duration;
    int unique_program_id;
    int avail_num;
    int avails_expected;
} SCTE35SpliceInsert;

// used to signal splice events
typedef struct SCTE35Time {
    SCTE35SpliceTime time;
} SCTE35Time;

/*
typedef struct SCTE35SpliceSchedule {
    int dummy;
} SCTE35SpliceSchedule;*/

typedef struct SCTE35AvailDescriptor {
    uint32_t provider_avail_id; 
} SCTE35AvailDescriptor;

typedef struct SCTE35DTMFDescriptor {
    uint8_t preroll;
    uint8_t dtmf_count;
    uint8_t reserved;
    uint8_t dtmf_char[256];
} SCTE35DTMFDescriptor;

typedef struct SCTE35SegDescriptor {
    uint32_t segmentation_event_id;
    int segmentation_event_cancel_indicator;
    int reserved_1;
    int program_segmentation_flag;
    int segmentation_duration_flag;
    int delivery_not_restricted_flag;
    int web_delivery_allowed_flag;
    int no_regional_blackout_flag;
    int archive_allowed_flag;
    int device_restrictions;
    int reserved_2;
    int component_count;
    SCTE35SpliceComponent component_info[MAX_SCTE35_SPLICE_COMPONENTS];
    uint64_t segmentation_duration;
    uint8_t segmentation_upid_type;
    uint8_t segmentation_upid_length;
    uint8_t segmentation_upid[SPLICE_DESCRIPTOR_DATA_MAX_SIZE];
    uint8_t segmentation_type_id;
    uint8_t segment_num;
    uint8_t segments_expected;
    uint8_t sub_segment_num;
    uint8_t sub_segments_expected;
} SCTE35SegDescriptor;

typedef struct SCTE35TimeDescriptor{
    uint64_t TAI_seconds;
    uint64_t TAI_ns;
    uint16_t UTC_offset;    
} SCTE35TimeDescriptor;

typedef struct SCTE35SpliceDesc {
    uint8_t splice_descriptor_tag;
    uint8_t descriptor_length;
    uint32_t identifier;
    
    union payload {
        SCTE35AvailDescriptor avail_desc;
	SCTE35DTMFDescriptor dtmp_desc;
	SCTE35SegDescriptor seg_desc;
	SCTE35TimeDescriptor time_desc;
	uint8_t data[SPLICE_DESCRIPTOR_DATA_MAX_SIZE];
    } payload;
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
    int64_t cur_packet_num;
    int64_t last_pcr_packet_num;
    int64_t last_pcr;
    int64_t next_pcr_packet_num;
    int64_t next_pcr;
    int64_t cur_pcr;

    union cmd {
        SCTE35SpliceInsert insert;
        //SCTE35SpliceSchedule schedule;
        SCTE35Time time_signal; 
    } cmd;

    int descriptor_loop_length;
    SCTE35SpliceDesc splice_descriptor;
    int num_alignment_bytes;
    uint32_t e_crc;
    uint32_t crc;
} SCTE35ParseSection;

void scte35_parse_init(SCTE35ParseSection *scte35_ptr);
int parse_insert(unsigned char *cmd, unsigned char *table, SCTE35ParseSection *scte35_ptr);
int parse_splice_descriptor(unsigned char *field, unsigned char *table, SCTE35SpliceDesc *splice_desc);
int parse_time_signal(unsigned char *cmd, unsigned char *table, SCTE35ParseSection *scte35_ptr);

#endif
