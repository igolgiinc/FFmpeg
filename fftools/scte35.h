#ifndef _SCTE35_H_
#define _SCTE35_H_

#define SPLICE_INFO_FIXED_SIZE 14
#define MAX_SCTE35_SPLICE_COMPONENTS 10

enum {
    SCTE35_CMD_NULL = 0,
    SCTE35_CMD_SCHEDULE = 4,
    SCTE35_CMD_INSERT = 5,
    SCTE35_CMD_TIME_SIGNAL = 6,
    SCTE35_CMD_BW_RESERVATION = 7,
    SCTE35_CMD_PRIVATE_CMD = 0xff
};

typedef struct SCTE35BreakDuration {
    int auto_return;
    int reserved;
    uint64_t duration;
} SCTE35BreakDuration;

typedef struct SCTE35SpliceTime {
    int time_specified_flag;
    int reserved;
    uint64_t pts_time;
    int byte_offset;
} SCTE35SpliceTime;

typedef struct SCTE35SpliceComponent {
    int component_tag;
    SCTE35SpliceTime time;
} SCTE35SpliceComponent;

typedef struct SCTE35SpliceInsert {
    int splice_event_id;
    int splice_event_cancel_indicator;
    int reserved_1;
    int out_of_network_indicator;
    int program_splice_flag;
    int duration_flag;
    int splice_immediate_flag;
    int reserved_2;
    SCTE35SpliceTime splice_time;
    int component_count;
    SCTE35SpliceComponent component_info[MAX_SCTE35_SPLICE_COMPONENTS];
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
    SCTE35SpliceDesc splice_descriptor;
    int num_alignment_bytes;
    uint32_t e_crc;
    uint32_t crc;
} SCTE35ParseSection;

static void scte35_parse_init(SCTE35ParseSection *scte35_ptr)
{
    scte35_ptr->table_id = -1;
    scte35_ptr->section_syntax_indicator = -1;
    scte35_ptr->private_indicator = -1;
    scte35_ptr->reserved = 0;
    scte35_ptr->section_length = -1;
    scte35_ptr->protocol_version = -1;
    scte35_ptr->encryption_algorithm = -1;
    scte35_ptr->pts_adjustment = 0;
    scte35_ptr->cw_index = -1;
    scte35_ptr->tier = -1;
    scte35_ptr->splice_command_length = -1;
    scte35_ptr->splice_command_type = -1;
    scte35_ptr->descriptor_loop_length = -1;
    scte35_ptr->num_alignment_bytes = -1;
    scte35_ptr->e_crc = 0;
    scte35_ptr->crc = 0;
}

static int parse_insert(unsigned char *cmd, unsigned char *table,
		        SCTE35ParseSection *scte35_ptr)
{
    int i;
    int nr;
    unsigned char *pdat = cmd;
    unsigned short unique_program_id;
    SCTE35SpliceComponent splice_component;
    SCTE35SpliceInsert *pcmd = &(scte35_ptr->cmd.insert);
    pcmd->splice_event_id = ((((uint32_t)*(pdat)) & 0xff) << 24) + 
	                    ((((uint32_t)*(pdat + 1)) & 0xff) << 16) + 
			    ((((uint32_t)*(pdat + 2)) & 0xff) << 8) + 
			    (((uint32_t)*(pdat + 3)) & 0xff);
    pdat += 4;
    pcmd->splice_event_cancel_indicator = (*pdat >> 7) & 1;
    pcmd->reserved_1 = (*pdat >> 0) & 0x7f;
    pdat++;
    
    if (pcmd->splice_event_cancel_indicator == 0) {
        pcmd->out_of_network_indicator = (*pdat >> 7) & 1;
	pcmd->program_splice_flag = (*pdat >> 5) & 1;
	pcmd->duration_flag = (*pdat >> 5) & 1;
	pcmd->splice_immediate_flag = (*pdat >> 4) & 1;
	pcmd->reserved_2 = *pdat & 0xf;
	pdat++;		
    }

    return 0;
}

#endif
