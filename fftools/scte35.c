#include "scte35.h"
#include "cmdutils.h"

// clears values in break duration struct
static void scte35_break_init(SCTE35BreakDuration *ptr)
{
    ptr->auto_return = -1;
    ptr->reserved = 0;
    ptr->duration = -1;
}

// clears values in splice_time struct
static void scte35_splice_time_init(SCTE35SpliceTime *ptr)
{
    ptr->time_specified_flag = -1;
    ptr->reserved = 0;
    ptr->pts_time = 0;
    ptr->byte_offset = -1;
}

// clears values in splice_insert struct
static void scte35_insert_init(SCTE35SpliceInsert *ptr)
{
    scte35_break_init(&ptr->break_duration);
    scte35_splice_time_init(&ptr->time);

    ptr->splice_event_id = -1;
    ptr->splice_event_cancel_indicator = -1;
    ptr->reserved_1 = -1;
    ptr->out_of_network_indicator = -1;
    ptr->program_splice_flag = -1;
    ptr->duration_flag = -1;
    ptr->splice_immediate_flag = -1;
    ptr->reserved_2 = -1;   
    ptr->component_count = -1;
    ptr->unique_program_id = -1;
    ptr->avail_num = -1;
    ptr->avails_expected = -1;
}

// clears values in splice_parse struct
void scte35_parse_init(SCTE35ParseSection *scte35_ptr)
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

// parses packet to fill splice_time struct
static int parse_splice_time(unsigned char *field, unsigned char *table,
		                    SCTE35SpliceTime *splice_time)
{
    unsigned char *pdat = field;
    splice_time->byte_offset = pdat - table;
    splice_time->time_specified_flag = (*pdat >> 7) & 1;
    splice_time->reserved = (*pdat >> 1) & 0x3f;

    if (splice_time->time_specified_flag == 1) {
        splice_time->pts_time = ((((uint64_t)*pdat) & 0x01) << 32) +
		                ((((uint64_t)*(pdat + 1)) & 0xff) << 24) + 
				(((uint64_t)*(pdat + 2) & 0xff) << 16) +
				(((uint64_t)*(pdat + 3) & 0xff) << 8) +
				(((uint64_t)*(pdat + 4) & 0xff) << 0);
	pdat += 5;
    }else{
        splice_time->pts_time = 0;
    }

    return pdat - field;
}

// parses packet to fill splice_component struct
static int parse_component(unsigned char *field, unsigned char *table, 
		            SCTE35SpliceComponent *scte35_ptr)
{
    int nr;
    unsigned char *pdat = field;

    scte35_ptr->component_tag = *pdat++;
    nr = parse_splice_time(pdat, table, &scte35_ptr->time);
    pdat += nr;

    return pdat - field;
}

// parses packet to fill parse_break_duration struct
static int parse_break_duration(unsigned char *field, unsigned char *table, 
		                SCTE35BreakDuration *break_duration)
{
    unsigned char *pdat = field;
    break_duration->auto_return = (*pdat >> 7) & 1;
    break_duration->reserved = (*pdat >> 1) & 0x3f;
    break_duration->duration = ((((uint64_t)*pdat) & 0x01) << 32) +
		               ((((uint64_t)*(pdat + 1)) & 0xff) << 24) + 
			       (((uint64_t)*(pdat + 2) & 0xff) << 16) +
			       (((uint64_t)*(pdat + 3) & 0xff) << 8) +
			       (((uint64_t)*(pdat + 4) & 0xff) << 0);
    pdat += 5;
 
    return pdat - field;
}

// parses packet to fill out SCTE35ParseSection struct
int parse_insert(unsigned char *cmd, unsigned char *table,
		        SCTE35ParseSection *scte35_ptr)
{

    int i;
    int nr;
    unsigned char *pdat = cmd;
    unsigned short unique_program_id;
    SCTE35SpliceComponent splice_component;
    SCTE35SpliceInsert *pcmd = &(scte35_ptr->cmd.insert);
    scte35_insert_init(pcmd);
    pcmd->splice_event_id = ((((uint32_t)*(pdat)) & 0xff) << 24) + 
	                    ((((uint32_t)*(pdat + 1)) & 0xff) << 16) + 
			    ((((uint32_t)*(pdat + 2)) & 0xff) << 8) + 
			    (((uint32_t)*(pdat + 3)) & 0xff);
    pdat += 4;
    pcmd->splice_event_cancel_indicator = (*pdat >> 7) & 1;
    pcmd->reserved_1 = *pdat & 0x7f;
    pdat++;
    
    if (pcmd->splice_event_cancel_indicator == 0) {
        pcmd->out_of_network_indicator = (*pdat >> 7) & 1;
	pcmd->program_splice_flag = (*pdat >> 6) & 1;
	pcmd->duration_flag = (*pdat >> 5) & 1;
	pcmd->splice_immediate_flag = (*pdat >> 4) & 1;
	pcmd->reserved_2 = *pdat & 0xf;
	pdat++;

        if (pcmd->program_splice_flag == 1 && pcmd->splice_immediate_flag == 0)
	{
	    nr = parse_splice_time(pdat, table, &pcmd->time);
	    pdat += nr;
	}

	/*if (pcmd->program_splice_flag == 0) 
        {
            pcmd->component_count = *pdat++;
	    for (i = 0; i < pcmd->component_count; i++) {
	        nr = parse_component(pdat, table, &splice_component);
		pdat += nr;
		if (i < MAX_SCTE35_SPLICE_COMPONENTS)
		    pcmd->component_info[i] = splice_component;
	    }	    
	}*/

        if (pcmd->duration_flag) {
	    nr = parse_break_duration(pdat, table, &pcmd->break_duration);
	    pdat += nr;
	}	
    }

    unique_program_id = *pdat++;
    unique_program_id <<= 8;
    unique_program_id = unique_program_id + *pdat++;
    pcmd->unique_program_id = unique_program_id;
    pcmd->avail_num = *pdat++;
    pcmd->avails_expected = *pdat++;

    return pdat - cmd;
}
