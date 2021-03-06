/*
 * DECT S-Format messages
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup ie_sfmt S-Format encoded IEs
 *
 * Raw S-Format encoded IE construction and parsing functions. Usually the
 * libdect user deals only with readily parsed IE structures representing the
 * S-Format encoded IEs, however the CLMS service carries raw encoded IEs.
 *
 * @{
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <asm/byteorder.h>

#include <libdect.h>
#include <identities.h>
#include <utils.h>
#include <s_fmt.h>
#include <lce.h>

#define sfmt_debug(fmt, args...) \
	dect_debug(DECT_DEBUG_SFMT, fmt, ## args)

static const struct dect_trans_tbl dect_repeat_indicators[] = {
	TRANS_TBL(DECT_IE_LIST_NORMAL,		"Non prioritized list"),
	TRANS_TBL(DECT_IE_LIST_PRIORITIZED,	"Prioritized list"),
};

static void dect_sfmt_dump_repeat_indicator(const struct dect_ie_common *_ie)
{
	const struct dect_ie_list *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\trepeat indicator: %s\n",
		   dect_val2str(dect_repeat_indicators, buf, ie->type));
}

static int dect_sfmt_parse_repeat_indicator(const struct dect_handle *dh,
					    struct dect_ie_common **ie,
					    const struct dect_sfmt_ie *src)
{
	struct dect_ie_list *dst = (struct dect_ie_list *)ie;

	dst->type = src->data[0] & DECT_SFMT_IE_FIXED_VAL_MASK;
	switch (dst->type) {
	case DECT_IE_LIST_NORMAL:
	case DECT_IE_LIST_PRIORITIZED:
		return 0;
	default:
		sfmt_debug("invalid list type\n");
		return -1;
	}
}

static int dect_sfmt_build_repeat_indicator(struct dect_sfmt_ie *dst,
					    const struct dect_ie_common *ie)
{
	struct dect_ie_list *src = dect_ie_container(src, ie);

	dst->data[0] = src->type;
	return 0;
}

static int dect_sfmt_parse_empty_single_octet(const struct dect_handle *dh,
					      struct dect_ie_common **ie,
					      const struct dect_sfmt_ie *src)
{
	return 0;
}

static int dect_sfmt_build_empty_single_octet(struct dect_sfmt_ie *dst,
					      const struct dect_ie_common *ie)
{
	dst->data[0] = 0;
	return 0;
}

static const struct dect_trans_tbl dect_call_classes[] = {
	TRANS_TBL(DECT_CALL_CLASS_LIA_SERVICE_SETUP,		"LiA service call setup"),
	TRANS_TBL(DECT_CALL_CLASS_MESSAGE,			"message call"),
	TRANS_TBL(DECT_CALL_CLASS_DECT_ISDN,			"DECT/ISDN IIP"),
	TRANS_TBL(DECT_CALL_CLASS_NORMAL,			"normal call"),
	TRANS_TBL(DECT_CALL_CLASS_INTERNAL,			"internal call"),
	TRANS_TBL(DECT_CALL_CLASS_EMERGENCY,			"emergency call"),
	TRANS_TBL(DECT_CALL_CLASS_SERVICE,			"service call"),
	TRANS_TBL(DECT_CALL_CLASS_EXTERNAL_HO,			"external handover call"),
	TRANS_TBL(DECT_CALL_CLASS_SUPPLEMENTARY_SERVICE,	"supplementary service call"),
	TRANS_TBL(DECT_CALL_CLASS_QA_M,				"QA&M call"),
};

static const struct dect_trans_tbl dect_basic_services[] = {
	TRANS_TBL(DECT_SERVICE_BASIC_SPEECH_DEFAULT,		"basic speech default attributes"),
	TRANS_TBL(DECT_SERVICE_DECT_GSM_IWP,			"DECT GSM IWP profile"),
	TRANS_TBL(DECT_SERVICE_UMTS_IWP,			"DECT UMTS IWP"),
	TRANS_TBL(DECT_SERVICE_LRMS,				"LRMS (E-profile) service"),
	TRANS_TBL(DECT_SERVICE_GSM_IWP_SMS,			"GSM IWP SMS"),
	TRANS_TBL(DECT_SERVICE_WIDEBAND_SPEECH,			"Wideband speech"),
	TRANS_TBL(DECT_SERVICE_SUOTA_CLASS_4_DPRS_MANAGEMENT,	"SUOTA, Class 4 DPRS management, default attributes"),
	TRANS_TBL(DECT_SERVICE_SUOTA_CLASS_3_DPRS_MANAGEMENT,	"SUOTA, Class 3 DPRS management, default attributes"),
	TRANS_TBL(DECT_SERVICE_OTHER,				"Other"),
};

static void dect_sfmt_dump_basic_service(const struct dect_ie_common *_ie)
{
	const struct dect_ie_basic_service *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tcall class: %s\n",
		   dect_val2str(dect_call_classes, buf, ie->class));
	sfmt_debug("\tservice: %s\n",
		   dect_val2str(dect_basic_services, buf, ie->service));
}

static int dect_sfmt_parse_basic_service(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_basic_service *dst = dect_ie_container(dst, *ie);

	dst->class   = src->data[1] >> DECT_BASIC_SERVICE_CALL_CLASS_SHIFT;
	dst->service = src->data[1] & DECT_BASIC_SERVICE_SERVICE_MASK;
	return 0;
}

static int dect_sfmt_build_basic_service(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *ie)
{
	struct dect_ie_basic_service *src = dect_ie_container(src, ie);

	dst->data[1]  = src->class << DECT_BASIC_SERVICE_CALL_CLASS_SHIFT;
	dst->data[1] |= src->service;
	return 0;
}

static void dect_sfmt_dump_display(const struct dect_ie_common *_ie)
{
	const struct dect_ie_display *ie = dect_ie_container(ie, _ie);

	dect_hexdump(DECT_DEBUG_SFMT, "\tInfo", ie->info, ie->len);
}

static int dect_sfmt_parse_single_display(const struct dect_handle *dh,
					  struct dect_ie_common **ie,
					  const struct dect_sfmt_ie *src)
{
	struct dect_ie_display *dst = dect_ie_container(dst, *ie);

	dst->info[0] = src->data[1];
	dst->len     = 1;
	return 0;
}

static int dect_sfmt_build_single_display(struct dect_sfmt_ie *dst,
					  const struct dect_ie_common *src)
{
	struct dect_ie_display *ie = dect_ie_container(ie, src);

	dst->data[1] = ie->info[0];
	return 0;
}

static void dect_sfmt_dump_keypad(const struct dect_ie_common *_ie)
{
	const struct dect_ie_keypad *ie = dect_ie_container(ie, _ie);

	dect_hexdump(DECT_DEBUG_SFMT, "\tInfo", ie->info, ie->len);
}

static int dect_sfmt_parse_single_keypad(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_keypad *dst = dect_ie_container(dst, *ie);

	dst->info[0] = src->data[1];
	dst->len = 1;
	return 0;
}

static int dect_sfmt_build_single_keypad(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *src)
{
	struct dect_ie_keypad *ie = dect_ie_container(ie, src);

	dst->data[1] = ie->info[0];
	return 0;
}

static const struct dect_trans_tbl dect_info_type_parameters[] = {
	TRANS_TBL(DECT_INFO_LOCATE_SUGGEST,				"Locate suggest"),
	TRANS_TBL(DECT_INFO_ACCESS_RIGHTS_MODIFY_SUGGEST,		"Access rights modify suggest"),
	TRANS_TBL(DECT_INFO_PP_AUTHENTICATION_FAILURE,			"PP authentication failure"),
	TRANS_TBL(DECT_INFO_DYNAMIC_PARAMETERS_ALLOCATION,		"Dynamic parameters allocation"),
	TRANS_TBL(DECT_INFO_EXTERNAL_HO_PARAMETERS,			"External handover parameters"),
	TRANS_TBL(DECT_INFO_LOCATION_AREA,				"Location area"),
	TRANS_TBL(DECT_INFO_HANDOVER_REFERENCE,				"Handover reference"),
	TRANS_TBL(DECT_INFO_MF_PSCN_SYNCHRONIZED_HANDOVER_CANDIATE,	"Multiframe/PSCN synchronized ext. handover candidate"),
	TRANS_TBL(DECT_INFO_EXT_HANDOVER_CANDIDATE,			"Ext. handover candidate"),
	TRANS_TBL(DECT_INFO_MF_SYNCHRONIZED_HANDOVER_CANDIATE,		"Multiframe synchronized ext. handover candidate"),
	TRANS_TBL(DECT_INFO_MF_PSCN_MFN_SYNCHRONIZED_HANDOVER_CANDIATE,	"Multiframe/PSCN/MFN synchronized ext. handover candidate"),
	TRANS_TBL(DECT_INFO_NON_SYNCHRONIZED_HANDOVER_CANDIDATE,	"Non synchronized ext. handover candidate"),
	TRANS_TBL(DECT_INFO_OLD_FIXED_PART_IDENTITY,			"Old fixed part identity"),
	TRANS_TBL(DECT_INFO_OLD_NETWORK_ASSIGNED_IDENTITY,		"Old network assigned identity"),
	TRANS_TBL(DECT_INFO_OLD_NETWORK_ASSIGNED_LOCATION_AREA,		"Old network assigned location area"),
	TRANS_TBL(DECT_INFO_OLD_NETWORK_ASSIGNED_HANDOVER_REFERENCE,	"Old network assigend handover reference"),
	TRANS_TBL(DECT_INFO_BILLING,					"Billing"),
	TRANS_TBL(DECT_INFO_DEBITING,					"Debiting"),
	TRANS_TBL(DECT_INFO_CK_TRANSFER,				"CK transfer"),
	TRANS_TBL(DECT_INFO_HANDOVER_FAILED_REVERSION,			"Handover failed, reversion to old channel"),
	TRANS_TBL(DECT_INFO_QA_M_CALL,					"QA&M call"),
	TRANS_TBL(DECT_INFO_DISTRIBUTED_COMMUNICATION_DOWNLOAD,		"Distributed Communication Download"),
	TRANS_TBL(DECT_INFO_ETHERNET_ADDRESS,				"Ethernet address"),
	TRANS_TBL(DECT_INFO_TOKEN_RING_ADDRESS,				"Token Ring address"),
	TRANS_TBL(DECT_INFO_IPV4_ADDRESS,				"IPv4 address"),
	TRANS_TBL(DECT_INFO_IPV6_ADDRESS,				"IPv6 address"),
	TRANS_TBL(DECT_INFO_IDENTITY_ALLOCATION,			"Identity allocation"),
};

static void dect_sfmt_dump_info_type(const struct dect_ie_common *_ie)
{
	const struct dect_ie_info_type *ie = dect_ie_container(ie, _ie);
	unsigned int i;
	char buf[128];

	for (i = 0; i < ie->num; i++)
		sfmt_debug("\tparameter type[%u]: %s\n", i,
			   dect_val2str(dect_info_type_parameters, buf, ie->type[i]));
}

static int dect_sfmt_build_info_type(struct dect_sfmt_ie *dst,
				     const struct dect_ie_common *src)
{
	struct dect_ie_info_type *ie = dect_ie_container(ie, src);
	unsigned int n = 2, i;

	for (i = 0; i < ie->num; i++)
		dst->data[n++] = ie->type[i];
	dst->data[n - 1] |= DECT_OCTET_GROUP_END;
	dst->len = n;
	return 0;
}

static int dect_sfmt_parse_info_type(const struct dect_handle *dh,
				     struct dect_ie_common **ie,
				     const struct dect_sfmt_ie *src)
{
	struct dect_ie_info_type *dst = dect_ie_container(dst, *ie);
	unsigned int n = 2;

	while (n < src->len) {
		dst->type[dst->num++] = src->data[n] & ~DECT_OCTET_GROUP_END;
		if (src->data[n] & DECT_OCTET_GROUP_END)
			break;
		n++;

		if (dst->num == array_size(dst->type))
			break;
	}
	return 0;
}

static const struct dect_trans_tbl dect_identity_groups[] = {
	TRANS_TBL(DECT_IDENTITY_PORTABLE_IDENTITY,		"portable identity"),
	TRANS_TBL(DECT_IDENTITY_NETWORK_ASSIGNED_IDENTITY,	"network assigned identity"),
	TRANS_TBL(DECT_IDENTITY_FIXED_IDENTITY,			"fixed identity"),
	TRANS_TBL(DECT_IDENTITY_APPLICATION_ASSIGNED,		"application assigned identity"),
	TRANS_TBL(DECT_IDENTITY_PROPRIETARY,			"proprietary"),
};

static const struct dect_trans_tbl dect_portable_identity_types[] = {
	TRANS_TBL(DECT_PORTABLE_ID_TYPE_IPUI,			"IPUI"),
	TRANS_TBL(DECT_PORTABLE_ID_TYPE_IPEI,			"IPEI"),
	TRANS_TBL(DECT_PORTABLE_ID_TYPE_TPUI,			"TPUI"),
};

static const struct dect_trans_tbl dect_fixed_identity_types[] = {
	TRANS_TBL(DECT_FIXED_ID_TYPE_ARI,			"ARI"),
	TRANS_TBL(DECT_FIXED_ID_TYPE_ARI_RPN,			"ARI/RPN"),
	TRANS_TBL(DECT_FIXED_ID_TYPE_ARI_WRS,			"ARI/WRS"),
	TRANS_TBL(DECT_FIXED_ID_TYPE_PARK,			"PARK"),
};

static const struct dect_trans_tbl dect_nwk_identity_types[] = {
	TRANS_TBL(DECT_NWK_ID_TYPE_TMSI,			"TMSI"),
	TRANS_TBL(DECT_NWK_ID_TYPE_PROPRIETARY,			"Proprietary"),
};

static void dect_sfmt_dump_identity_type(const struct dect_ie_common *_ie)
{
	const struct dect_ie_identity_type *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tidentity group: %s\n",
		   dect_val2str(dect_identity_groups, buf, ie->group));

	switch (ie->group) {
	case DECT_IDENTITY_PORTABLE_IDENTITY:
		sfmt_debug("\tidentity type: %s\n",
			   dect_val2str(dect_portable_identity_types, buf, ie->type));
		break;
	case DECT_IDENTITY_FIXED_IDENTITY:
		sfmt_debug("\tidentity type: %s\n",
			   dect_val2str(dect_fixed_identity_types, buf, ie->type));
		break;
	case DECT_IDENTITY_NETWORK_ASSIGNED_IDENTITY:
		sfmt_debug("\tidentity type: %s\n",
			   dect_val2str(dect_nwk_identity_types, buf, ie->type));
		break;
	case DECT_IDENTITY_APPLICATION_ASSIGNED:
	case DECT_IDENTITY_PROPRIETARY:
	default:
		sfmt_debug("\tidentity type: %u\n", ie->type);
		break;
	}
}

static int dect_sfmt_build_identity_type(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *src)
{
	struct dect_ie_identity_type *ie = dect_ie_container(ie, src);

	dst->data[2] = ie->group | DECT_OCTET_GROUP_END;
	dst->data[3] = ie->type  | DECT_OCTET_GROUP_END;
	dst->len = 4;
	return 0;
}

static int dect_sfmt_parse_identity_type(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_identity_type *dst = dect_ie_container(dst, *ie);

	dst->group = src->data[2] & ~DECT_OCTET_GROUP_END;
	dst->type  = src->data[3] & ~DECT_OCTET_GROUP_END;
	return 0;
}

static const struct dect_trans_tbl dect_release_reasons[] = {
	TRANS_TBL(DECT_RELEASE_NORMAL,				"normal"),
	TRANS_TBL(DECT_RELEASE_UNEXPECTED_MESSAGE,		"unexpected message"),
	TRANS_TBL(DECT_RELEASE_UNKNOWN_TRANSACTION_IDENTIFIER,	"unknown transaction identifier"),
	TRANS_TBL(DECT_RELEASE_MANDATORY_IE_MISSING,		"mandatory IE missing"),
	TRANS_TBL(DECT_RELEASE_INVALID_IE_CONTENTS,		"invalid IE contents"),
	TRANS_TBL(DECT_RELEASE_INCOMPATIBLE_SERVICE,		"incompatible service"),
	TRANS_TBL(DECT_RELEASE_SERVICE_NOT_IMPLEMENTED,		"service not implemented"),
	TRANS_TBL(DECT_RELEASE_NEGOTIATION_NOT_SUPPORTED,	"negotiation not supported"),
	TRANS_TBL(DECT_RELEASE_INVALID_IDENTITY,		"invalid identity"),
	TRANS_TBL(DECT_RELEASE_AUTHENTICATION_FAILED,		"authentication failed"),
	TRANS_TBL(DECT_RELEASE_UNKNOWN_IDENTITY,		"unknown identity"),
	TRANS_TBL(DECT_RELEASE_NEGOTIATION_FAILED,		"negotiation failed"),
	TRANS_TBL(DECT_RELEASE_TIMER_EXPIRY,			"timer expiry"),
	TRANS_TBL(DECT_RELEASE_PARTIAL_RELEASE,			"partial release"),
	TRANS_TBL(DECT_RELEASE_UNKNOWN,				"unknown"),
	TRANS_TBL(DECT_RELEASE_USER_DETACHED,			"user detached"),
	TRANS_TBL(DECT_RELEASE_USER_NOT_IN_RANGE,		"user not in range"),
	TRANS_TBL(DECT_RELEASE_USER_UNKNOWN,			"user unknown"),
	TRANS_TBL(DECT_RELEASE_USER_ALREADY_ACTIVE,		"user already active"),
	TRANS_TBL(DECT_RELEASE_USER_BUSY,			"user busy"),
	TRANS_TBL(DECT_RELEASE_USER_REJECTION,			"user rejection"),
	TRANS_TBL(DECT_RELEASE_USER_CALL_MODIFY,		"user call modify"),
	TRANS_TBL(DECT_RELEASE_EXTERNAL_HANDOVER_NOT_SUPPORTED,	"external HO not supported"),
	TRANS_TBL(DECT_RELEASE_NETWORK_PARAMETERS_MISSING,	"network parameters missing"),
	TRANS_TBL(DECT_RELEASE_EXTERNAL_HANDOVER_RELEASE,	"external HO release"),
	TRANS_TBL(DECT_RELEASE_OVERLOAD,			"overload"),
	TRANS_TBL(DECT_RELEASE_INSUFFICIENT_RESOURCES,		"insufficient resources"),
	TRANS_TBL(DECT_RELEASE_INSUFFICIENT_BEARERS_AVAILABLE,	"insufficient bearers available"),
	TRANS_TBL(DECT_RELEASE_IWU_CONGESTION,			"IWU congestion"),
	TRANS_TBL(DECT_RELEASE_SECURITY_ATTACK_ASSUMED,		"Security attack assumed"),
	TRANS_TBL(DECT_RELEASE_ENCRYPTION_ACTIVATION_FAILED,	"Encryption activation failed"),
	TRANS_TBL(DECT_RELEASE_REKEYING_FAILED,			"Rekeying failed"),
};

static void dect_sfmt_dump_release_reason(const struct dect_ie_common *_ie)
{
	const struct dect_ie_release_reason *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\trelease reason: %s\n",
		   dect_val2str(dect_release_reasons, buf, ie->reason));
}

static int dect_sfmt_parse_release_reason(const struct dect_handle *dh,
					  struct dect_ie_common **ie,
					  const struct dect_sfmt_ie *src)
{
	struct dect_ie_release_reason *dst = dect_ie_container(dst, *ie);

	dst->reason = src->data[1];
	return 0;
}

static int dect_sfmt_build_release_reason(struct dect_sfmt_ie *dst,
					  const struct dect_ie_common *ie)
{
	struct dect_ie_release_reason *src = dect_ie_container(src, ie);

	dst->data[1] = src->reason;
	return 0;
}

static const struct dect_trans_tbl dect_signal_codes[] = {
	TRANS_TBL(DECT_SIGNAL_DIAL_TONE_ON,				"ring tone on"),
	TRANS_TBL(DECT_SIGNAL_RING_BACK_TONE_ON,			"ring-back tone on"),
	TRANS_TBL(DECT_SIGNAL_INTERCEPT_TONE_ON,			"intercept tone on"),
	TRANS_TBL(DECT_SIGNAL_NETWORK_CONGESTION_TONE_ON,		"network congestion tone on"),
	TRANS_TBL(DECT_SIGNAL_BUSY_TONE_ON,				"busy tone on"),
	TRANS_TBL(DECT_SIGNAL_CONFIRM_TONE_ON,				"confirm tone on"),
	TRANS_TBL(DECT_SIGNAL_ANSWER_TONE_ON,				"answer tone on"),
	TRANS_TBL(DECT_SIGNAL_CALL_WAITING_TONE_ON,			"call waiting tone on"),
	TRANS_TBL(DECT_SIGNAL_OFF_HOOK_WARNING_TONE_ON,			"off-hook warning tone on"),
	TRANS_TBL(DECT_SIGNAL_NEGATIVE_ACKNOWLEDGEMENT_TONE,		"negative acknowledgement tone"),
	TRANS_TBL(DECT_SIGNAL_TONES_OFF,				"tones off"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_PATTERN_0,	"ring pattern 0"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_PATTERN_1,	"ring pattern 1"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_PATTERN_2,	"ring pattern 2"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_PATTERN_3,	"ring pattern 3"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_PATTERN_4,	"ring pattern 4"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_PATTERN_5,	"ring pattern 5"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_PATTERN_6,	"ring pattern 6"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_PATTERN_7,	"ring pattern 7"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_CONTINUOUS,	"ring continuous"),
	TRANS_TBL(DECT_SIGNAL_ALERTING_BASE + DECT_ALERTING_OFF,	"ring off"),
};

static void dect_sfmt_dump_signal(const struct dect_ie_common *_ie)
{
	struct dect_ie_signal *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tsignal: %s\n", dect_val2str(dect_signal_codes, buf, ie->code));
}

static int dect_sfmt_parse_signal(const struct dect_handle *dh,
				  struct dect_ie_common **ie,
				  const struct dect_sfmt_ie *src)
{
	struct dect_ie_signal *dst = dect_ie_container(dst, *ie);

	dst->code = src->data[1];
	return 0;
}

static int dect_sfmt_build_signal(struct dect_sfmt_ie *dst,
				  const struct dect_ie_common *src)
{
	struct dect_ie_signal *ie = dect_ie_container(ie, src);

	dst->data[1] = ie->code;
	return 0;
}

static int dect_sfmt_parse_timer_restart(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_timer_restart *dst = dect_ie_container(dst, *ie);

	dst->code = src->data[1];
	switch (dst->code) {
	case DECT_TIMER_RESTART:
	case DECT_TIMER_STOP:
		return 0;
	default:
		return -1;
	}
}

static void dect_sfmt_dump_portable_identity(const struct dect_ie_common *_ie)
{
	const struct dect_ie_portable_identity *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\ttype: %s\n",
		   dect_val2str(dect_portable_identity_types, buf, ie->type));

	switch (ie->type) {
	case DECT_PORTABLE_ID_TYPE_IPUI:
	case DECT_PORTABLE_ID_TYPE_IPEI:
		return dect_dump_ipui(&ie->ipui);
	case DECT_PORTABLE_ID_TYPE_TPUI:
		return dect_dump_tpui(&ie->tpui);
	}
}

static int dect_sfmt_parse_portable_identity(const struct dect_handle *dh,
					     struct dect_ie_common **ie,
					     const struct dect_sfmt_ie *src)
{
	struct dect_ie_portable_identity *dst = dect_ie_container(dst, *ie);
	uint8_t len;

	/* Empty <<PORTABLE-IDENTITY>> IEs are of length 2 */
	if (src->len == 2)
		return 0;

	if (src->len < DECT_IE_PORTABLE_IDENTITY_MIN_SIZE)
		return -1;

	if (!(src->data[2] & DECT_OCTET_GROUP_END))
		return -1;
	dst->type = src->data[2] & ~DECT_OCTET_GROUP_END;

	if (!(src->data[3] & DECT_OCTET_GROUP_END))
		return -1;
	len = src->data[3] & ~DECT_OCTET_GROUP_END;

	switch (dst->type) {
	case DECT_PORTABLE_ID_TYPE_IPUI:
	case DECT_PORTABLE_ID_TYPE_IPEI:
		if (!dect_parse_ipui(&dst->ipui, src->data + 4, len))
			sfmt_debug("parsing failed\n");
		return 0;
	case DECT_PORTABLE_ID_TYPE_TPUI:
		return 0;
	default:
		sfmt_debug("invalid type %u\n", dst->type);
		return -1;
	}
}

static int dect_sfmt_build_portable_identity(struct dect_sfmt_ie *dst,
					     const struct dect_ie_common *src)
{
	const struct dect_ie_portable_identity *ie = dect_ie_container(ie, src);
	uint32_t tpui;
	uint8_t len;

	switch (ie->type) {
	case DECT_PORTABLE_ID_TYPE_IPUI:
	case DECT_PORTABLE_ID_TYPE_IPEI:
		len = dect_build_ipui(&dst->data[4], &ie->ipui);
		if (len == 0)
			return -1;
		break;
	case DECT_PORTABLE_ID_TYPE_TPUI:
		tpui = dect_build_tpui(&ie->tpui);
		dst->data[6] = tpui;
		dst->data[5] = tpui >> 8;
		dst->data[4] = tpui >> 16;
		len = 20;
		break;
	default:
		return -1;
	}

	dst->data[3] = DECT_OCTET_GROUP_END | len;
	dst->data[2] = DECT_OCTET_GROUP_END | ie->type;
	dst->len = 4 + div_round_up(len, 8);
	return 0;
}

static void dect_sfmt_dump_fixed_identity(const struct dect_ie_common *_ie)
{
	const struct dect_ie_fixed_identity *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\ttype: %s\n",
		   dect_val2str(dect_fixed_identity_types, buf, ie->type));

	switch (ie->type) {
	case DECT_FIXED_ID_TYPE_ARI:
		return dect_dump_ari(&ie->ari);
	case DECT_FIXED_ID_TYPE_PARK:
		return dect_dump_ari(&ie->ari);
	case DECT_FIXED_ID_TYPE_ARI_RPN:
		dect_dump_ari(&ie->ari);
		sfmt_debug("\tRPN: %u\n", ie->rpn);
		return;
	case DECT_FIXED_ID_TYPE_ARI_WRS:
		return dect_dump_ari(&ie->ari);
	}
}

static int dect_sfmt_parse_fixed_identity(const struct dect_handle *dh,
					  struct dect_ie_common **ie,
					  const struct dect_sfmt_ie *src)
{
	struct dect_ie_fixed_identity *dst = dect_ie_container(dst, *ie);
	uint8_t len, ari_len;
	uint64_t ari;

	if (src->len < DECT_IE_FIXED_IDENTITY_MIN_SIZE)
		return -1;

	if (!(src->data[2] & DECT_OCTET_GROUP_END))
		return -1;
	dst->type = src->data[2] & ~DECT_OCTET_GROUP_END;

	if (!(src->data[3] & DECT_OCTET_GROUP_END))
		return -1;
	len = src->data[3] & ~DECT_OCTET_GROUP_END;

	ari  = __be64_to_cpu(*(__be64 *)&src->data[4]);
	ari_len = dect_parse_ari(&dst->ari, ari << 1);
	if (ari_len == 0)
		return -1;

	switch (dst->type) {
	case DECT_FIXED_ID_TYPE_ARI:
	case DECT_FIXED_ID_TYPE_PARK:
		return ari_len + 1 == len;
	case DECT_FIXED_ID_TYPE_ARI_RPN:
	case DECT_FIXED_ID_TYPE_ARI_WRS:
		return 0;
	default:
		sfmt_debug("invalid type %u\n", dst->type);
		return -1;
	}
}

static int dect_sfmt_build_fixed_identity(struct dect_sfmt_ie *dst,
					  const struct dect_ie_common *ie)
{
	struct dect_ie_fixed_identity *src = dect_ie_container(src, ie);
	uint64_t ari;

	ari = dect_build_ari(&src->ari) >> 1;
	dst->data[8] = ari >> 24;
	dst->data[7] = ari >> 32;
	dst->data[6] = ari >> 40;
	dst->data[5] = ari >> 48;
	dst->data[4] = ari >> 56;
	dst->data[3] = DECT_OCTET_GROUP_END | (DECT_ARC_A_LEN + 1);
	dst->data[2] = DECT_OCTET_GROUP_END | src->type;
	dst->len = 9;
	return 0;
}

static void dect_sfmt_dump_location_area(const struct dect_ie_common *_ie)
{
	const struct dect_ie_location_area *ie = dect_ie_container(ie, _ie);

	sfmt_debug("\ttype: %x level: %u\n", ie->type, ie->level);
}

static int dect_sfmt_parse_location_area(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_location_area *dst = dect_ie_container(dst, *ie);

	dst->type  = (src->data[2] & DECT_LOCATION_AREA_TYPE_MASK) >>
		     DECT_LOCATION_AREA_TYPE_SHIFT;
	dst->level = (src->data[2] & DECT_LOCATION_LEVEL_MASK);
	return 0;
}

static int dect_sfmt_build_location_area(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *ie)
{
	struct dect_ie_location_area *src = dect_ie_container(src, ie);

	dst->data[2]  = src->type << DECT_LOCATION_AREA_TYPE_SHIFT;
	dst->data[2] |= src->level;
	dst->len = 3;
	return 0;
}

static const struct dect_trans_tbl dect_auth_algs[] = {
	TRANS_TBL(DECT_AUTH_DSAA,			"DSAA"),
	TRANS_TBL(DECT_AUTH_GSM,			"GSM"),
	TRANS_TBL(DECT_AUTH_UMTS,			"UMTS"),
	TRANS_TBL(DECT_AUTH_PROPRIETARY,		"proprietary"),
};

static const struct dect_trans_tbl dect_auth_key_types[] = {
	TRANS_TBL(DECT_KEY_USER_AUTHENTICATION_KEY,	"User authentication key"),
	TRANS_TBL(DECT_KEY_USER_PERSONAL_IDENTITY,	"User personal identity"),
	TRANS_TBL(DECT_KEY_AUTHENTICATION_CODE,		"Authentication code"),
};

static void dect_sfmt_dump_allocation_type(const struct dect_ie_common *_ie)
{
	const struct dect_ie_allocation_type *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tauthentication algorithm: %s\n",
		   dect_val2str(dect_auth_algs, buf, ie->auth_id));
	sfmt_debug("\tauthentication key number: %u\n", ie->auth_key_num);
	sfmt_debug("\tauthentication code number: %u\n", ie->auth_code_num);
}

static int dect_sfmt_parse_allocation_type(const struct dect_handle *dh,
					   struct dect_ie_common **ie,
					   const struct dect_sfmt_ie *src)
{
	struct dect_ie_allocation_type *dst = dect_ie_container(dst, *ie);

	dst->auth_id = src->data[2];
	dst->auth_key_num  = (src->data[3] & 0xf0) >> 4;
	dst->auth_code_num = (src->data[3] & 0x0f);
	return 0;
}

static int dect_sfmt_build_allocation_type(struct dect_sfmt_ie *dst,
					   const struct dect_ie_common *ie)
{
	struct dect_ie_allocation_type *src = dect_ie_container(src, ie);

	dst->data[2]  = src->auth_id;
	dst->data[3]  = src->auth_key_num << 4;
	dst->data[3] |= src->auth_code_num;
	dst->len = 4;
	return 0;
}

static const struct dect_trans_tbl dect_auth_flags[] = {
	TRANS_TBL(DECT_AUTH_FLAG_INC,		"INC"),
	TRANS_TBL(DECT_AUTH_FLAG_DEF,		"DEF"),
	TRANS_TBL(DECT_AUTH_FLAG_TXC,		"TXC"),
	TRANS_TBL(DECT_AUTH_FLAG_UPC,		"UPC"),
};

static void dect_sfmt_dump_auth_type(const struct dect_ie_common *_ie)
{
	const struct dect_ie_auth_type *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tauthentication algorithm: %s\n",
		   dect_val2str(dect_auth_algs, buf, ie->auth_id));
	sfmt_debug("\tauthentication key type: %s\n",
		   dect_val2str(dect_auth_key_types, buf, ie->auth_key_type));
	sfmt_debug("\tauthentication key number: %u\n", ie->auth_key_num);
	sfmt_debug("\tcipher key number: %u\n", ie->cipher_key_num);
	sfmt_debug("\tflags: %d\n", ie->flags);
//		   dect_flags2str(dect_auth_flags, buf, ie->flags));
	if (ie->flags & DECT_AUTH_FLAG_DEF)
		sfmt_debug("\tdefault cipher key index: %u\n",
			   ie->defck_index);
}

static int dect_sfmt_parse_auth_type(const struct dect_handle *dh,
				     struct dect_ie_common **ie,
				     const struct dect_sfmt_ie *src)
{
	struct dect_ie_auth_type *dst = dect_ie_container(dst, *ie);
	uint8_t n = 2;

	dst->auth_id = src->data[n++];
	if (dst->auth_id == DECT_AUTH_PROPRIETARY)
		dst->proprietary_auth_id = src->data[n]++;

	dst->auth_key_type  = (src->data[n] & 0xf0) >> 4;
	dst->auth_key_num   = (src->data[n] & 0x0f);
	n++;

	dst->flags	    = src->data[n] & 0xf0;
	dst->cipher_key_num = src->data[n] & 0x0f;
	n++;

	/* Octets 5a and 5b are only present if the DEF flag is set */
	if (dst->flags & DECT_AUTH_FLAG_DEF)
		dst->defck_index = src->data[n] << 8 |
				   src->data[n + 1];

	return 0;
}

static int dect_sfmt_build_auth_type(struct dect_sfmt_ie *dst,
				     const struct dect_ie_common *ie)
{
	struct dect_ie_auth_type *src = dect_ie_container(src, ie);
	uint8_t n = 2;

	dst->data[n++] = src->auth_id;
	if (src->auth_id == DECT_AUTH_PROPRIETARY)
		dst->data[n++] = 0;

	dst->data[n]  = src->auth_key_type << 4;
	dst->data[n] |= src->auth_key_num;
	n++;

	dst->data[n]  = src->flags;
	dst->data[n] |= src->cipher_key_num;
	n++;

	/* Octets 5a and 5b are only present if the DEF flag is set */
	if (src->flags & DECT_AUTH_FLAG_DEF) {
		dst->data[n++] = src->defck_index >> 8;
		dst->data[n++] = src->defck_index;
	}

	dst->len = n;
	return 0;
}

static void dect_sfmt_dump_auth_value(const struct dect_ie_common *_ie)
{
	const struct dect_ie_auth_value *ie = dect_ie_container(ie, _ie);

	sfmt_debug("\tvalue: %.16" PRIx64 "\n", ie->value);
}

static int dect_sfmt_parse_auth_value(const struct dect_handle *dh,
				      struct dect_ie_common **ie,
				      const struct dect_sfmt_ie *src)
{
	struct dect_ie_auth_value *dst = dect_ie_container(dst, *ie);

	if (src->len != sizeof(dst->value) + 2)
		return -1;
	dst->value = *(uint64_t *)&src->data[2];
	return 0;
}

static int dect_sfmt_build_auth_value(struct dect_sfmt_ie *dst,
				      const struct dect_ie_common *ie)
{
	struct dect_ie_auth_value *src = dect_ie_container(src, ie);

	*(uint64_t *)&dst->data[2] = src->value;
	dst->len = sizeof(src->value) + 2;
	return 0;
}

static void dect_sfmt_dump_auth_res(const struct dect_ie_common *_ie)
{
	const struct dect_ie_auth_res *ie = dect_ie_container(ie, _ie);

	sfmt_debug("\tvalue: %.8x\n", ie->value);
}

static int dect_sfmt_parse_auth_res(const struct dect_handle *dh,
				    struct dect_ie_common **ie,
				    const struct dect_sfmt_ie *src)
{
	struct dect_ie_auth_res *dst = dect_ie_container(dst, *ie);

	if (src->len != sizeof(dst->value) + 2)
		return -1;
	dst->value = *(uint32_t *)&src->data[2];
	return 0;
}

static int dect_sfmt_build_auth_res(struct dect_sfmt_ie *dst,
				    const struct dect_ie_common *ie)
{
	struct dect_ie_auth_res *src = dect_ie_container(src, ie);

	*(uint32_t *)&dst->data[2] = src->value;
	dst->len = sizeof(src->value) + 2;
	return 0;
}

static const struct dect_trans_tbl dect_service_change_modes[] = {
	TRANS_TBL(DECT_SERVICE_CHANGE_NONE,				"None"),
	TRANS_TBL(DECT_SERVICE_CHANGE_CONNECTION_REVERSAL,		"Connection Reversal"),
	TRANS_TBL(DECT_SERVICE_CHANGE_BANDWIDTH,			"Bandwidth change"),
	TRANS_TBL(DECT_SERVICE_CHANGE_MODULATION,			"Modulation scheme change"),
	TRANS_TBL(DECT_SERVICE_CHANGE_REROUTING,			"Rerouting of U-plane links"),
	TRANS_TBL(DECT_SERVICE_CHANGE_BANDWIDTH_PLUS_MODULATION,	"Bandwidth plus modulation scheme change"),
	TRANS_TBL(DECT_SERVICE_CHANGE_REROUTING_PLUS_BANDWIDTH,		"Rerouting plus bandwidth change"),
	TRANS_TBL(DECT_SERVICE_CHANGE_BANDWIDTH_OR_MODULATION,		"Bandwidth or modulation scheme change"),
	TRANS_TBL(DECT_SERVICE_CHANGE_SUSPEND,				"Suspend"),
	TRANS_TBL(DECT_SERVICE_CHANGE_RESUME,				"Resume"),
	TRANS_TBL(DECT_SERVICE_CHANGE_VOICE_DATA_TO_DATA,		"Voice/data change to data"),
	TRANS_TBL(DECT_SERVICE_CHANGE_VOICE_DATA_TO_VOICE,		"Voice/data change to voice"),
	TRANS_TBL(DECT_SERVICE_CHANGE_IWU_ATTRIBUTES,			"IWU attribute change"),
	TRANS_TBL(DECT_SERVICE_CHANGE_AUDIO_CODEC,			"Audio Codec change"),
	TRANS_TBL(DECT_SERVICE_CHANGE_BASIC_SERVICE_AND_IWU_ATTRIBUTES,	"Profile/Basic service and IWU attributes change"),
};

static void dect_sfmt_dump_service_change_info(const struct dect_ie_common *_ie)
{
	const struct dect_ie_service_change_info *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tmaster: %d\n", ie->master);
	sfmt_debug("\tmode: %s\n", dect_val2str(dect_service_change_modes, buf, ie->mode));
}

static int dect_sfmt_parse_service_change_info(const struct dect_handle *dh,
					       struct dect_ie_common **ie,
					       const struct dect_sfmt_ie *src)
{
	struct dect_ie_service_change_info *dst = dect_ie_container(dst, *ie);

	dst->master = src->data[2] & 0x40;
	dst->mode   = src->data[2] & 0x0f;
	return 0;
}

static const struct dect_trans_tbl dect_cipher_algs[] = {
	TRANS_TBL(DECT_CIPHER_STANDARD_1,		"DECT Standard Cipher 1"),
	TRANS_TBL(DECT_CIPHER_GPRS_NO_CIPHERING,	"GPRS ciphering not used"),
	TRANS_TBL(DECT_CIPHER_GRPS_GEA_1,		"GPRS GEA/1"),
	TRANS_TBL(DECT_CIPHER_GRPS_GEA_2,		"GPRS GEA/2"),
	TRANS_TBL(DECT_CIPHER_GRPS_GEA_3,		"GPRS GEA/3"),
	TRANS_TBL(DECT_CIPHER_GRPS_GEA_4,		"GPRS GEA/4"),
	TRANS_TBL(DECT_CIPHER_GRPS_GEA_5,		"GPRS GEA/5"),
	TRANS_TBL(DECT_CIPHER_GRPS_GEA_6,		"GPRS GEA/6"),
	TRANS_TBL(DECT_CIPHER_GRPS_GEA_7,		"GPRS GEA/7"),
	TRANS_TBL(DECT_CIPHER_ESC_TO_PROPRIETARY,	"Escape to proprietary"),
};

static const struct dect_trans_tbl dect_cipher_key_types[] = {
	TRANS_TBL(DECT_CIPHER_DERIVED_KEY,		"derived"),
	TRANS_TBL(DECT_CIPHER_STATIC_KEY,		"static"),
};

static void dect_sfmt_dump_cipher_info(const struct dect_ie_common *_ie)
{
	const struct dect_ie_cipher_info *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tenable: %u\n", ie->enable);
	sfmt_debug("\tcipher algorithm: %s\n",
		   dect_val2str(dect_cipher_algs, buf, ie->cipher_alg_id));
	sfmt_debug("\tcipher key type: %s\n",
		   dect_val2str(dect_cipher_key_types, buf, ie->cipher_key_type));
	sfmt_debug("\tcipher key num: %u\n", ie->cipher_key_num);
}

static int dect_sfmt_parse_cipher_info(const struct dect_handle *dh,
				       struct dect_ie_common **ie,
				       const struct dect_sfmt_ie *src)
{
	struct dect_ie_cipher_info *dst = dect_ie_container(dst, *ie);

	if (src->len != 4)
		return -1;

	dst->enable		= src->data[2] & 0x80;
	dst->cipher_alg_id	= src->data[2] & 0x7f;
	dst->cipher_key_type	= (src->data[3] & 0xf0) >> 4;
	dst->cipher_key_num	= src->data[3] & 0x0f;
	return 0;
}

static int dect_sfmt_build_cipher_info(struct dect_sfmt_ie *dst,
				       const struct dect_ie_common *ie)
{
	struct dect_ie_cipher_info *src = dect_ie_container(src, ie);

	dst->data[2]  = src->enable ? 0x80 : 0;
	dst->data[2] |= src->cipher_alg_id;
	dst->data[3]  = src->cipher_key_type << 4;
	dst->data[3] |= src->cipher_key_num | 0x8;
	dst->len = 4;
	return 0;
}

static const struct dect_trans_tbl dect_facility_services[] = {
	TRANS_TBL(DECT_FACILITY_SS,			"supplementary services"),
};

static void dect_sfmt_dump_facility(const struct dect_ie_common *_ie)
{
	const struct dect_ie_facility *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tservice discriminator: %s\n",
		   dect_val2str(dect_facility_services, buf, ie->service));
	dect_hexdump(DECT_DEBUG_SFMT, "\tComponents", ie->components, ie->len);
}

static int dect_sfmt_parse_facility(const struct dect_handle *dh,
				    struct dect_ie_common **ie,
				    const struct dect_sfmt_ie *src)
{
	struct dect_ie_facility *dst = dect_ie_container(dst, *ie);

	dst->service = src->data[2] & 0x1f;
	dst->len = src->len - 3;
	if (dst->len > array_size(dst->components))
		return -1;
	memcpy(dst->components, src->data + 3, dst->len);
	return 0;
}

static const struct dect_trans_tbl dect_locations[] = {
	TRANS_TBL(DECT_LOCATION_USER,					"user"),
	TRANS_TBL(DECT_LOCATION_PRIVATE_NETWORK_SERVING_LOCAL_USER,	"private network serving the local user"),
	TRANS_TBL(DECT_LOCATION_PUBLIC_NETWORK_SERVING_LOCAL_USER,	"public network serving the local user"),
	TRANS_TBL(DECT_LOCATION_PUBLIC_NETWORK_SERVING_REMOTE_USER,	"public network serving the remote user"),
	TRANS_TBL(DECT_LOCATION_PRIVATE_NETWORK_SERVING_REMOTE_USER,	"private network serving the remote user"),
	TRANS_TBL(DECT_LOCATION_INTERNATIONAL_NETWORK,			"international network"),
	TRANS_TBL(DECT_LOCATION_NETWORK_BEYONG_INTERWORKING_POINT,	"network beyond interworking point"),
	TRANS_TBL(DECT_LOCATION_NOT_APPLICABLE,				"not applicable"),
};

static const struct dect_trans_tbl dect_progress_descriptions[] = {
	TRANS_TBL(DECT_PROGRESS_NOT_END_TO_END_ISDN,			"Call is not end-to-end ISDN, further call progress info may be available in-band"),
	TRANS_TBL(DECT_PROGRESS_DESTINATION_ADDRESS_NON_ISDN,		"Destination address is non-ISDN"),
	TRANS_TBL(DECT_PROGRESS_ORIGINATION_ADDRESS_NON_ISDN,		"Origination address is non-ISDN"),
	TRANS_TBL(DECT_PROGRESS_CALL_RETURNED_TO_ISDN,			"Call has returned to the ISDN"),
	TRANS_TBL(DECT_PROGRESS_SERVICE_CHANGE,				"Service change has occurred"),
	TRANS_TBL(DECT_PROGRESS_INBAND_INFORMATION_NOW_AVAILABLE,	"In-band information or appropriate pattern now available"),
	TRANS_TBL(DECT_PROGRESS_INBAND_INFORMATION_NOT_AVAILABLE,	"In-band information not available"),
	TRANS_TBL(DECT_PROGRESS_END_TO_END_ISDN,			"Call is end-to-end PLMN/ISDN"),
};

static void dect_sfmt_dump_progress_indicator(const struct dect_ie_common *_ie)
{
	const struct dect_ie_progress_indicator *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tLocation: %s\n",
		   dect_val2str(dect_locations, buf, ie->location));
	sfmt_debug("\tProgress description: %s\n",
		   dect_val2str(dect_progress_descriptions, buf, ie->progress));
}

static int dect_sfmt_parse_progress_indicator(const struct dect_handle *dh,
					      struct dect_ie_common **ie,
					      const struct dect_sfmt_ie *src)
{
	struct dect_ie_progress_indicator *dst = dect_ie_container(dst, *ie);

	dst->location = src->data[2] & DECT_SFMT_IE_PROGRESS_INDICATOR_LOCATION_MASK;
	dst->progress = src->data[3] & ~DECT_OCTET_GROUP_END;
	return 0;
}

static int dect_sfmt_build_progress_indicator(struct dect_sfmt_ie *dst,
					      const struct dect_ie_common *ie)
{
	struct dect_ie_progress_indicator *src = dect_ie_container(src, ie);

	dst->data[3] = DECT_OCTET_GROUP_END | src->progress;
	dst->data[2] = DECT_OCTET_GROUP_END | src->location;
	dst->len = 4;
	return 0;
}

static const struct dect_trans_tbl dect_time_date_codings[] = {
	TRANS_TBL(DECT_TIME_DATE_TIME,		"Time"),
	TRANS_TBL(DECT_TIME_DATE_DATE,		"Date"),
	TRANS_TBL(DECT_TIME_DATE_TIME_AND_DATE,	"Time and Date"),
};

static const struct dect_trans_tbl dect_time_date_interpretations[] = {
	TRANS_TBL(DECT_TIME_DATE_CURRENT,	"current time/date"),
	TRANS_TBL(DECT_TIME_DATE_DURATION,	"duration"),
};

static void dect_sfmt_dump_time_date(const struct dect_ie_common *_ie)
{
	const struct dect_ie_time_date *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tCoding: %s\n",
		   dect_val2str(dect_time_date_codings, buf, ie->coding));
	sfmt_debug("\tInterpetation: %s\n",
		   dect_val2str(dect_time_date_interpretations, buf,
				ie->interpretation));

	if (ie->coding & 0x2)
		sfmt_debug("\tDate: %u%u.%u%u.20%u%u\n",
			   ie->day >> 4, ie->day & 0x0f,
			   ie->month >> 4, ie->month & 0x0f,
			   ie->year >> 4, ie->year & 0xf);

	if (ie->coding & 0x1)
		sfmt_debug("\tTime: %u%u:%u%u:%u%u %+dmin\n",
			   ie->hour >> 4, ie->hour & 0x0f,
			   ie->minute >> 4, ie->minute & 0x0f,
			   ie->second >> 4, ie->second & 0xf,
			   15 * (ie->timezone & 0x8 ? -1 : 1) *
			   ((10 * (ie->timezone >> 4) & 0x7) +
			    (ie->timezone & 0x0f)));
}

static int dect_sfmt_parse_time_date(const struct dect_handle *dh,
				     struct dect_ie_common **ie,
				     const struct dect_sfmt_ie *src)
{
	struct dect_ie_time_date *dst = dect_ie_container(dst, *ie);
	unsigned int n;

	dst->coding         = src->data[2] >> 6;
	dst->interpretation = src->data[2] & 0x3f;
	n = 3;

	if (dst->coding & 0x2) {
		dst->year     = src->data[n++];
		dst->month    = src->data[n++];
		dst->day      = src->data[n++];
	}

	if (dst->coding & 0x1) {
		dst->hour     = src->data[n++];
		dst->minute   = src->data[n++];
		dst->second   = src->data[n++];
		dst->timezone = src->data[n++];
	}
	return 0;
}

static int dect_sfmt_build_time_date(struct dect_sfmt_ie *dst,
				     const struct dect_ie_common *ie)
{
	struct dect_ie_time_date *src = dect_ie_container(src, ie);
	unsigned int n;

	dst->data[2]  = src->coding << 6;
	dst->data[2] |= src->interpretation;
	n = 3;

	if (src->coding & 0x2) {
		dst->data[n++] = src->year;
		dst->data[n++] = src->month;
		dst->data[n++] = src->day;
	}

	if (src->coding & 0x1) {
		dst->data[n++] = src->hour;
		dst->data[n++] = src->minute;
		dst->data[n++] = src->second;
		dst->data[n++] = src->timezone;
	}

	dst->len = n;
	return 0;
}

static int dect_sfmt_parse_multi_display(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_display *dst = dect_ie_container(dst, *ie);

	dst->len = src->len - 2;
	if (dst->len > array_size(dst->info))
		return -1;
	memcpy(dst->info, src->data + 2, dst->len);
	return 0;
}

static int dect_sfmt_build_multi_display(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *ie)
{
	struct dect_ie_display *src = dect_ie_container(src, ie);

	memcpy(dst->data + 2, src->info, src->len);
	dst->len = src->len + 2;
	return 0;
}

static int dect_sfmt_parse_multi_keypad(const struct dect_handle *dh,
					struct dect_ie_common **ie,
					const struct dect_sfmt_ie *src)
{
	struct dect_ie_keypad *dst = dect_ie_container(dst, *ie);

	dst->len = src->len - 2;
	if (dst->len > array_size(dst->info))
		return -1;
	memcpy(dst->info, src->data + 2, dst->len);
	return 0;
}

static int dect_sfmt_build_multi_keypad(struct dect_sfmt_ie *dst,
					const struct dect_ie_common *ie)
{
	struct dect_ie_keypad *src = dect_ie_container(src, ie);

	memcpy(dst->data + 2, src->info, src->len);
	dst->len = src->len + 2;
	return 0;
}

static const struct dect_trans_tbl dect_features[] = {
	TRANS_TBL(DECT_FEATURE_REGISTER_RECALL,			"register recall"),
	TRANS_TBL(DECT_FEATURE_EXTERNAL_HO_SWITCH,		"external handover switch"),
	TRANS_TBL(DECT_FEATURE_QUEUE_ENTRY_REQUEST,		"queue entry request"),
	TRANS_TBL(DECT_FEATURE_INDICATION_OF_SUBSCRIBER_NUMBER,	"indication of subscriber number"),
	TRANS_TBL(DECT_FEATURE_FEATURE_KEY,			"feature key"),
	TRANS_TBL(DECT_FEATURE_SPECIFIC_LINE_SELECTION,		"specific line selection"),
	TRANS_TBL(DECT_FEATURE_SPECIFIC_TRUNK_SELECTION,	"specific trunk carrier selection"),
	TRANS_TBL(DECT_FEATURE_ECHO_CONTROL,			"echo control"),
	TRANS_TBL(DECT_FEATURE_COST_INFORMATION,		"cost information"),
};

static void dect_sfmt_dump_feature_activate(const struct dect_ie_common *_ie)
{
	const struct dect_ie_feature_activate *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tfeature: %s\n", dect_val2str(dect_features, buf, ie->feature));
}

static int dect_sfmt_parse_feature_activate(const struct dect_handle *dh,
					    struct dect_ie_common **ie,
					    const struct dect_sfmt_ie *src)
{
	struct dect_ie_feature_activate *dst = dect_ie_container(dst, *ie);

	dst->feature = src->data[2] & ~DECT_OCTET_GROUP_END;
	return 0;
}

static int dect_sfmt_build_feature_activate(struct dect_sfmt_ie *dst,
					    const struct dect_ie_common *ie)
{
	struct dect_ie_feature_activate *src = dect_ie_container(src, ie);

	dst->data[2]  = src->feature;
	dst->data[2] |= DECT_OCTET_GROUP_END;
	dst->len = 3;
	return 0;
}

static void dect_sfmt_dump_feature_indicate(const struct dect_ie_common *_ie)
{
	const struct dect_ie_feature_indicate *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tfeature: %s\n", dect_val2str(dect_features, buf, ie->feature));
	sfmt_debug("\tstatus: %x\n", ie->status);
}

static int dect_sfmt_parse_feature_indicate(const struct dect_handle *dh,
					    struct dect_ie_common **ie,
					    const struct dect_sfmt_ie *src)
{
	struct dect_ie_feature_indicate *dst = dect_ie_container(dst, *ie);

	dst->feature = src->data[2] & ~DECT_OCTET_GROUP_END;
	dst->status  = src->data[3];
	return 0;
}

static const struct dect_trans_tbl dect_network_parameter_discriminators[] = {
	TRANS_TBL(DECT_NETWORK_PARAMETER_APPLICATION_ASSIGNED,			"Application assigned"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_DEVICE_NAME,				"Device name"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_HO_REFERENCE_NOT_ACQUIRED,		"Handover reference not requied"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_HO_REFERENCE_PRIVATE_NETWORK,		"Handover reference, private network"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_HO_REFERENCE_GSM_NETWORK,		"Handover reference, GSM network"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_HO_REFERENCE_PUBLIC_NETWORK,		"Handover reference, public network"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_PROPRIETARY,				"Proprietary"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_HO_REFERENCE_REQUEST_GSM_NETWORK,	"Handover reference request, GSM network"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_HO_REFERENCE_UMTS_NETWORK,		"Handover reference, UMTS network"),
	TRANS_TBL(DECT_NETWORK_PARAMETER_HO_REFERENCE_REQUEST_UMTS_NETWORK,	"Handover reference request, UMTS network"),
};

static void dect_sfmt_dump_network_parameter(const struct dect_ie_common *_ie)
{
	const struct dect_ie_network_parameter *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tDiscriminator: %s\n",
		   dect_val2str(dect_network_parameter_discriminators,
			        buf, ie->discriminator));
	sfmt_debug("\tData: %.*s\n", ie->len, ie->data);
}

static int dect_sfmt_parse_network_parameter(const struct dect_handle *dh,
					     struct dect_ie_common **ie,
					     const struct dect_sfmt_ie *src)
{
	struct dect_ie_network_parameter *dst = dect_ie_container(dst, *ie);

	dst->discriminator = src->data[2];
	dst->len = src->len - 3;
	if (dst->len > array_size(dst->data))
		return -1;
	memcpy(dst->data, src->data + 3, dst->len);
	return 0;
}

static int dect_sfmt_build_network_parameter(struct dect_sfmt_ie *dst,
					     const struct dect_ie_common *ie)
{
	const struct dect_ie_network_parameter *src = dect_ie_container(src, ie);

	dst->data[2] = src->discriminator;
	memcpy(dst->data + 3, src->data, src->len);
	dst->len = src->len + 3;
	return 0;
}

static const struct dect_trans_tbl dect_reject_reasons[] = {
	TRANS_TBL(DECT_REJECT_TPUI_UNKNOWN,				"TPUI unknown"),
	TRANS_TBL(DECT_REJECT_IPUI_UNKNOWN,				"IPUI unknown"),
	TRANS_TBL(DECT_REJECT_NETWORK_ASSIGNED_IDENTITY_UNKNOWN,	"network assign identity unknown"),
	TRANS_TBL(DECT_REJECT_IPEI_NOT_ACCEPTED,			"IPEI not accepted"),
	TRANS_TBL(DECT_REJECT_IPUI_NOT_ACCEPTED,			"IPUI not accepted"),
	TRANS_TBL(DECT_REJECT_AUTHENTICATION_FAILED,			"authentication failed"),
	TRANS_TBL(DECT_REJECT_NO_AUTHENTICATION_ALGORITHM,		"no authentication algorithm"),
	TRANS_TBL(DECT_REJECT_AUTHENTICATION_ALGORITHM_NOT_SUPPORTED,	"authentication algorithm not supported"),
	TRANS_TBL(DECT_REJECT_AUTHENTICATION_KEY_NOT_SUPPORTED,		"authentication key not supported"),
	TRANS_TBL(DECT_REJECT_UPI_NOT_ENTERED,			 	"UPI not entered"),
	TRANS_TBL(DECT_REJECT_NO_CIPHER_ALGORITHM,			"no cipher algorithm"),
	TRANS_TBL(DECT_REJECT_CIPHER_ALGORITHM_NOT_SUPPORTED,		"cipher algorithm not supported"),
	TRANS_TBL(DECT_REJECT_CIPHER_KEY_NOT_SUPPORTED,			"cipher key not supported"),
	TRANS_TBL(DECT_REJECT_INCOMPATIBLE_SERVICE,			"incompatible service"),
	TRANS_TBL(DECT_REJECT_FALSE_LCE_REPLY,				"false LCE reply"),
	TRANS_TBL(DECT_REJECT_LATE_LCE_REPLY,				"late LCE reply"),
	TRANS_TBL(DECT_REJECT_INVALID_TPUI,				"invalid TPUI"),
	TRANS_TBL(DECT_REJECT_TPUI_ASSIGNMENT_LIMITS_UNACCEPTABLE,	"TPUI assignment limits unacceptable"),
	TRANS_TBL(DECT_REJECT_INSUFFICIENT_MEMORY,			"insufficient memory"),
	TRANS_TBL(DECT_REJECT_OVERLOAD,					"overload"),
	TRANS_TBL(DECT_REJECT_TEST_CALL_BACK_NORMAL_EN_BLOC,		"test callback - en-bloc dialing"),
	TRANS_TBL(DECT_REJECT_TEST_CALL_BACK_NORMAL_PIECEWISE,		"test callback - piecewise dialing"),
	TRANS_TBL(DECT_REJECT_TEST_CALL_BACK_EMERGENCY_EN_BLOC,		"emergency test callback - en-bloc dialing"),
	TRANS_TBL(DECT_REJECT_TEST_CALL_BACK_EMERGENCY_PIECEWISE,	"emergency test callback - piecewise dialing"),
	TRANS_TBL(DECT_REJECT_INVALID_MESSAGE,				"invalid message"),
	TRANS_TBL(DECT_REJECT_INFORMATION_ELEMENT_ERROR,		"information element error"),
	TRANS_TBL(DECT_REJECT_INVALID_INFORMATION_ELEMENT_CONTENTS,	"invalid information element contents"),
	TRANS_TBL(DECT_REJECT_TIMER_EXPIRY,				"timer expiry"),
	TRANS_TBL(DECT_REJECT_PLMN_NOT_ALLOWED,				"plmn not allowed"),
	TRANS_TBL(DECT_REJECT_LOCATION_AREA_NOT_ALLOWED,		"location area not allowed"),
	TRANS_TBL(DECT_REJECT_LOCATION_NATIONAL_ROAMING_NOT_ALLOWED,	"national roaming not allowed"),
};

static void dect_sfmt_dump_reject_reason(const struct dect_ie_common *_ie)
{
	struct dect_ie_reject_reason *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\treject reason: %s\n",
		   dect_val2str(dect_reject_reasons, buf, ie->reason));
}

static int dect_sfmt_parse_reject_reason(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_reject_reason *dst = dect_ie_container(dst, *ie);

	dst->reason = src->data[2];
	return 0;
}

static int dect_sfmt_build_reject_reason(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *ie)
{
	struct dect_ie_reject_reason *src = dect_ie_container(src, ie);

	dst->data[2] = src->reason;
	dst->len = 3;
	return 0;
}

static int dect_sfmt_parse_setup_capability(const struct dect_handle *dh,
					    struct dect_ie_common **ie,
					    const struct dect_sfmt_ie *src)
{
	struct dect_ie_setup_capability *dst = dect_ie_container(dst, *ie);

	dst->page_capability  = (src->data[2] & 0x3);
	dst->setup_capability = (src->data[2] & 0xc) >> 2;
	return 0;
}

static int dect_sfmt_build_setup_capability(struct dect_sfmt_ie *dst,
					    const struct dect_ie_common *ie)
{
	struct dect_ie_setup_capability *src = dect_ie_container(src, ie);

	dst->data[2]  = src->page_capability;
	dst->data[2] |= src->setup_capability << 2;
	dst->data[2] |= DECT_OCTET_GROUP_END;
	dst->len = 3;
	return 0;
}

static const struct dect_trans_tbl dect_display_capabilities[] = {
	TRANS_TBL(DECT_DISPLAY_CAPABILITY_NOT_APPLICABLE,	"not applicable"),
	TRANS_TBL(DECT_DISPLAY_CAPABILITY_NO_DISPLAY,		"no display"),
	TRANS_TBL(DECT_DISPLAY_CAPABILITY_NUMERIC,		"numeric"),
	TRANS_TBL(DECT_DISPLAY_CAPABILITY_NUMERIC_PLUS,		"numeric-plus"),
	TRANS_TBL(DECT_DISPLAY_CAPABILITY_ALPHANUMERIC,		"alphanumeric"),
	TRANS_TBL(DECT_DISPLAY_CAPABILITY_FULL_DISPLAY,		"full display"),
};

static const struct dect_trans_tbl dect_tone_capabilities[] = {
	TRANS_TBL(DECT_TONE_CAPABILITY_NOT_APPLICABLE,		"not applicable"),
	TRANS_TBL(DECT_TONE_CAPABILITY_NO_TONE,			"no tone"),
	TRANS_TBL(DECT_TONE_CAPABILITY_DIAL_TONE_ONLY,		"dial tone only"),
	TRANS_TBL(DECT_TONE_CAPABILITY_ITU_T_E182_TONES,	"ITU-T E.182 tones"),
	TRANS_TBL(DECT_TONE_CAPABILITY_COMPLETE_DECT_TONES,	"complete DECT tones"),
};

static const struct dect_trans_tbl dect_echo_parameters[] = {
	TRANS_TBL(DECT_ECHO_PARAMETER_NOT_APPLICABLE,		"not applicable"),
	TRANS_TBL(DECT_ECHO_PARAMETER_MINIMUM_TCLW,		"TCL > 34 dB"),
	TRANS_TBL(DECT_ECHO_PARAMETER_FULL_TCLW,		"TCL > 46 dB"),
	TRANS_TBL(DECT_ECHO_PARAMETER_VOIP_COMPATIBLE_TLCW,	"TCL > 55 dB"),
};

static const struct dect_trans_tbl dect_noise_rejection_capabilities[] = {
	TRANS_TBL(DECT_NOISE_REJECTION_NOT_APPLICABLE,		"not applicable"),
	TRANS_TBL(DECT_NOISE_REJECTION_NONE,			"none"),
	TRANS_TBL(DECT_NOISE_REJECTION_PROVIDED,		"provided"),
};

static const struct dect_trans_tbl dect_volume_ctrl_provisions[] = {
	TRANS_TBL(DECT_ADAPTIVE_VOLUME_NOT_APPLICABLE,		"not applicable"),
	TRANS_TBL(DECT_ADAPTIVE_VOLUME_PP_CONTROL_NONE,		"no PP adaptive volume control"),
	TRANS_TBL(DECT_ADAPTIVE_VOLUME_PP_CONTROL_USED,		"PP adaptive volume control"),
	TRANS_TBL(DECT_ADAPTIVE_VOLUME_FP_CONTROL_DISABLE,	"disable FP adaptive volume control"),
};

static const struct dect_trans_tbl dect_scrolling_behaviour[] = {
	TRANS_TBL(DECT_SCROLLING_NOT_SPECIFIED,			"not specified"),
	TRANS_TBL(DECT_SCROLLING_TYPE_1,			"type 1"),
	TRANS_TBL(DECT_SCROLLING_TYPE_2,			"type 2"),
};

static const struct dect_trans_tbl dect_slot_capabilities[] = {
	TRANS_TBL(DECT_SLOT_CAPABILITY_HALF_SLOT,		"half slot"),
	TRANS_TBL(DECT_SLOT_CAPABILITY_LONG_SLOT_640,		"long slot 640"),
	TRANS_TBL(DECT_SLOT_CAPABILITY_LONG_SLOT_672,		"long slot 672"),
	TRANS_TBL(DECT_SLOT_CAPABILITY_FULL_SLOT,		"full slot"),
	TRANS_TBL(DECT_SLOT_CAPABILITY_DOUBLE_SLOT,		"double slot"),
};

static const struct dect_trans_tbl dect_profile_indicators[] = {
	TRANS_TBL(DECT_PROFILE_DPRS_ASYMETRIC_BEARERS_SUPPORTED,	"DPRS asymetric bearers"),
	TRANS_TBL(DECT_PROFILE_DPRS_STREAM_SUPPORTED,			"DPRS Stream"),
	TRANS_TBL(DECT_PROFILE_LRMS_SUPPORTED,				"LRMS"),
	TRANS_TBL(DECT_PROFILE_ISDN_END_SYSTEM_SUPPORTED,		"ISDN End-system"),
	TRANS_TBL(DECT_PROFILE_DECT_GSM_INTERWORKING_PROFILE_SUPPORTED,"DECT/GSM interworking"),
	TRANS_TBL(DECT_PROFILE_GAP_SUPPORTED,				"GAP"),
	TRANS_TBL(DECT_PROFILE_CAP_SUPPORTED,				"CAP"),
	TRANS_TBL(DECT_PROFILE_RAP_1_PROFILE_SUPPORTED,			"RAP 1"),
	TRANS_TBL(DECT_PROFILE_UMTS_GSM_FACSIMILE_SUPPORTED,		"UMTS-GSM interworking - Facsimile service"),
	TRANS_TBL(DECT_PROFILE_UMTS_GSM_SMS_SERVICE_SUPPORTED,		"UMTS-GSM interworking - SMS service"),
	TRANS_TBL(DECT_PROFILE_UMTS_GSM_BEARER_SERVICE,			"UMTS-GSM interworking - bearer service"),
	TRANS_TBL(DECT_PROFILE_ISDN_IAP_SUPPORTED,			"ISDN Intermediate Access"),
	TRANS_TBL(DECT_PROFILE_DATA_SERVICES_PROFILE_D,			"Data Services Profile D"),
	TRANS_TBL(DECT_PROFILE_DPRS_FREL_SUPPORTED,			"DPRS FREL"),
	TRANS_TBL(DECT_PROFILE_TOKEN_RING_SUPPORTED,			"Token Ring"),
	TRANS_TBL(DECT_PROFILE_ETHERNET_SUPPORTED,			"Ethernet"),
	TRANS_TBL(DECT_PROFILE_MULTIPORT_CTA,				"Multiport CPA"),
	TRANS_TBL(DECT_PROFILE_DMAP_SUPPORTED,				"DMAP"),
	TRANS_TBL(DECT_PROFILE_SMS_OVER_LRMS_SUPPORTED,			"SMS over LRMS"),
	TRANS_TBL(DECT_PROFILE_WRS_SUPPORTED,				"WRS"),
	TRANS_TBL(DECT_PROFILE_DECT_GSM_DUAL_MODE_TERMINAL,		"DECT/GSM dual mode terminal"),
	TRANS_TBL(DECT_PROFILE_DPRS_SUPPORTED,				"DPRS"),
	TRANS_TBL(DECT_PROFILE_RAP_2_PROFILE_SUPPORTED,			"RAP 2"),
	TRANS_TBL(DECT_PROFILE_I_PQ_SERVICES_SUPPORTED,			"I_pq services"),
	TRANS_TBL(DECT_PROFILE_C_F_CHANNEL_SUPPORTED,			"C_f channel"),
	TRANS_TBL(DECT_PROFILE_V_24_SUPPORTED,				"V.24"),
	TRANS_TBL(DECT_PROFILE_PPP_SUPPORTED,				"PPP"),
	TRANS_TBL(DECT_PROFILE_IP_SUPPORTED,				"IP"),
	TRANS_TBL(DECT_PROFILE_8_LEVEL_A_FIELD_MODULATION,		"8-level A-field modulation"),
	TRANS_TBL(DECT_PROFILE_4_LEVEL_A_FIELD_MODULATION,		"4-level A-field modulation"),
	TRANS_TBL(DECT_PROFILE_2_LEVEL_A_FIELD_MODULATION,		"2-level A-field modulation"),
	TRANS_TBL(DECT_PROFILE_16_LEVEL_BZ_FIELD_MODULATION,		"16-level B/Z-field modulation"),
	TRANS_TBL(DECT_PROFILE_8_LEVEL_BZ_FIELD_MODULATION,		"8-level B/Z-field modulation"),
	TRANS_TBL(DECT_PROFILE_4_LEVEL_BZ_FIELD_MODULATION,		"4-level B/Z-field modulation"),
	TRANS_TBL(DECT_PROFILE_2_LEVEL_BZ_FIELD_MODULATION,		"2-level B/Z-field modulation"),
	TRANS_TBL(DECT_PROFILE_NO_EMISSION_MODE_SUPPORTED,		"no emission mode"),
	TRANS_TBL(DECT_PROFILE_PT_WITH_FAST_HOPPING_RADIO,		"fast hopping radio"),
	TRANS_TBL(DECT_PROFILE_G_F_CHANNEL_SUPPORTED,			"G_f channel"),
	TRANS_TBL(DECT_PROFILE_F_MMS_INTERWORKING_PROFILE_SUPPORTED,	"F-MMS Interworking"),
	TRANS_TBL(DECT_PROFILE_BASIC_ODAP_SUPPORTED,			"Basic ODAP"),
	TRANS_TBL(DECT_PROFILE_DECT_UMTS_INTERWORKING_GPRS_SUPPORTED,	"UMTS interworking - GPRS service"),
	TRANS_TBL(DECT_PROFILE_DECT_UMTS_INTERWORKING_PROFILE_SUPPORTED, "UMTS interworking"),
	TRANS_TBL(DECT_PROFILE_REKEYING_EARLY_ENCRYPTION_SUPPORTED,	"Rekeying and early encryption"),
	TRANS_TBL(DECT_PROFILE_HEADSET_MANAGEMENT_SUPPORTED,		"Headset management"),
	TRANS_TBL(DECT_PROFILE_NG_DECT_PART_3,				"DECT-NG part 3"),
	TRANS_TBL(DECT_PROFILE_NG_DECT_PART_1,				"DECT-NG part 1"),
	TRANS_TBL(DECT_PROFILE_64_LEVEL_BZ_FIELD_MODULATION,		"64-level B/Z-field modulation"),
};

static void dect_sfmt_dump_terminal_capability(const struct dect_ie_common *_ie)
{
	const struct dect_ie_terminal_capability *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tdisplay capability: %s\n",
		   dect_val2str(dect_display_capabilities, buf, ie->display));
	sfmt_debug("\ttone capability: %s\n",
		   dect_val2str(dect_tone_capabilities, buf, ie->tone));
	sfmt_debug("\techo parameters: %s\n",
		   dect_val2str(dect_echo_parameters, buf, ie->echo));
	sfmt_debug("\tnoise rejection capability: %s\n",
		   dect_val2str(dect_noise_rejection_capabilities, buf, ie->noise_rejection));
	sfmt_debug("\tadaptive volume control provision: %s\n",
		   dect_val2str(dect_volume_ctrl_provisions, buf, ie->volume_ctrl));
	sfmt_debug("\tslot capabilities: %d\n", ie->slot);
//		   dect_flags2str(dect_slot_capabilities, buf, ie->slot));
	sfmt_debug("\tdisplay memory: %u\n", ie->display_memory);
	sfmt_debug("\tdisplay lines: %u\n", ie->display_lines);
	sfmt_debug("\tdisplay columns: %u\n", ie->display_columns);
	sfmt_debug("\tscrolling behaviour: %s\n",
		   dect_val2str(dect_scrolling_behaviour, buf, ie->scrolling));
	sfmt_debug("\tprofile indicator: %d\n", ie->profile_indicator);
//		   dect_flags2str(dect_profile_indicators, buf, ie->profile_indicator));
	sfmt_debug("\tdisplay control: %x\n", ie->display_control);
	sfmt_debug("\tdisplay charsets: %x\n", ie->display_charsets);
}

static int dect_sfmt_parse_terminal_capability(const struct dect_handle *dh,
					       struct dect_ie_common **ie,
					       const struct dect_sfmt_ie *src)
{
	struct dect_ie_terminal_capability *dst = dect_ie_container(dst, *ie);
	uint8_t i, n = 2;

	/* Octet group 3 */
	dst->display = (src->data[n] & DECT_TERMINAL_CAPABILITY_DISPLAY_MASK);
	dst->tone    = (src->data[n] & DECT_TERMINAL_CAPABILITY_TONE_MASK) >>
		       DECT_TERMINAL_CAPABILITY_TONE_SHIFT;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->echo	     = (src->data[n] & DECT_TERMINAL_CAPABILITY_ECHO_MASK) >>
			       DECT_TERMINAL_CAPABILITY_ECHO_SHIFT;
	dst->noise_rejection = (src->data[n] & DECT_TERMINAL_CAPABILITY_NOISE_MASK) >>
			       DECT_TERMINAL_CAPABILITY_NOISE_SHIFT;
	dst->volume_ctrl     = (src->data[n] & DECT_TERMINAL_CAPABILITY_VOLUME_MASK);
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->slot = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->display_memory = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;
	dst->display_memory <<= 7;

	dst->display_memory += src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->display_lines   = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->display_columns = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->scrolling	     = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

group4:
	dst->profile_indicator = 0;
	for (i = 0; i < 8; i++) {
		dst->profile_indicator |=
			(uint64_t)(src->data[n] & ~DECT_OCTET_GROUP_END) <<
			(64 - 8 * (i + 1));
		if (src->data[n++] & DECT_OCTET_GROUP_END)
			goto group5;
	}

group5:
	dst->display_control = src->data[n] & 0x7;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group6;
	dst->display_charsets = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group6;

group6:
	/* Older equipment may not include octet group 6 */
	if (n == src->len)
		goto group7;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group7;
	if (!(src->data[n++] & DECT_OCTET_GROUP_END))
		return -1;

group7:
	return 0;
}

static int dect_sfmt_build_terminal_capability(struct dect_sfmt_ie *dst,
					       const struct dect_ie_common *ie)
{
	struct dect_ie_terminal_capability *src = dect_ie_container(src, ie);
	uint8_t i, n = 2;

	/* Octet group 3 */
	dst->data[n]    = src->display;
	dst->data[n++] |= src->tone << DECT_TERMINAL_CAPABILITY_TONE_SHIFT;

	dst->data[n]    = src->echo << DECT_TERMINAL_CAPABILITY_ECHO_SHIFT;
	dst->data[n]   |= src->noise_rejection << DECT_TERMINAL_CAPABILITY_NOISE_SHIFT;
	dst->data[n++] |= src->volume_ctrl;

	dst->data[n++]  = src->slot;
	dst->data[n++]  = src->display_memory >> 7;
	dst->data[n++]  = src->display_memory;
	dst->data[n++]  = src->display_lines;
	dst->data[n++]  = src->display_columns;
	dst->data[n]    = src->scrolling;
	dst->data[n++] |= DECT_OCTET_GROUP_END;

	/* Octet group 4 */
	for (i = 0; i < 8; i++) {
		dst->data[n] = src->profile_indicator >> (64 - 8 * (i + 1));
		if (!(src->profile_indicator & (~0ULL >> (64 - 8 * (i + 1))))) {
			dst->data[n++] |= DECT_OCTET_GROUP_END;
			break;
		}
		n++;
	}

	/* Octet group 5 */
	dst->data[n++]  = src->display_control;
	dst->data[n]    = src->display_charsets;
	dst->data[n++] |= DECT_OCTET_GROUP_END;

	dst->len = n;
	return 0;
}

static const struct dect_trans_tbl dect_number_types[] = {
	TRANS_TBL(DECT_NUMBER_TYPE_UNKNOWN,		"unknown"),
	TRANS_TBL(DECT_NUMBER_TYPE_INTERNATIONAL,	"international number"),
	TRANS_TBL(DECT_NUMBER_TYPE_NATIONAL,		"national number"),
	TRANS_TBL(DECT_NUMBER_TYPE_NETWORK_SPECIFIC,	"network specific number"),
	TRANS_TBL(DECT_NUMBER_TYPE_SUBSCRIBER,		"subscriber number"),
	TRANS_TBL(DECT_NUMBER_TYPE_ABBREVIATED,		"abbreviated number"),
	TRANS_TBL(DECT_NUMBER_TYPE_RESERVED,		"reserved"),
};

static const struct dect_trans_tbl dect_npis[] = {
	TRANS_TBL(DECT_NPI_UNKNOWN,			"unknown"),
	TRANS_TBL(DECT_NPI_ISDN_E164,			"ISDN/telephony plan E.164"),
	TRANS_TBL(DECT_NPI_DATA_PLAN_X121,		"data plan X.121"),
	TRANS_TBL(DECT_NPI_TCP_IP,			"TCP/IP address"),
	TRANS_TBL(DECT_NPI_NATIONAL_STANDARD,		"national standard plan"),
	TRANS_TBL(DECT_NPI_PRIVATE,			"private plan"),
	TRANS_TBL(DECT_NPI_SIP,				"SIP"),
	TRANS_TBL(DECT_NPI_INTERNET_CHARACTER_FORMAT,	"internet character format"),
	TRANS_TBL(DECT_NPI_LAN_MAC_ADDRESS,		"LAN MAC address"),
	TRANS_TBL(DECT_NPI_X400,			"X.400 address"),
	TRANS_TBL(DECT_NPI_PROFILE_SPECIFIC,		"profile specific identifier"),
	TRANS_TBL(DECT_NPI_RESERVED,			"reserved"),
};

static const struct dect_trans_tbl dect_presentation_indicators[] = {
	TRANS_TBL(DECT_PRESENTATION_ALLOWED,			"Presentation allowed"),
	TRANS_TBL(DECT_PRESENTATION_RESTRICTED,			"Presentation restricted"),
	TRANS_TBL(DECT_PRESENTATION_NOT_AVAILABLE,		"Name/Number not available"),
	TRANS_TBL(DECT_PRESENTATION_HANDSET_LOCATOR,		"Handset locator"),
};

static const struct dect_trans_tbl dect_alphabets[] = {
	TRANS_TBL(DECT_ALPHABET_STANDARD,			"DECT standard"),
	TRANS_TBL(DECT_ALPHABET_UTF8,				"UTF-8"),
	TRANS_TBL(DECT_ALPHABET_NETWORK_SPECIFIC,		"Network specific"),
};

static const struct dect_trans_tbl dect_screening_indicators[] = {
	TRANS_TBL(DECT_SCREENING_USER_PROVIDED_NOT_SCREENED,	"User-provided, not screened"),
	TRANS_TBL(DECT_SCREENING_USER_PROVIDED_VERIFIED_PASSED,	"User-provided, verified and passed"),
	TRANS_TBL(DECT_SCREENING_USER_PROVIDED_VERIFIED_FAILED,	"User-provided, verified and failed"),
	TRANS_TBL(DECT_SCREENING_NETWORK_PROVIDED,		"Network provided"),
};

static void dect_sfmt_dump_calling_party_number(const struct dect_ie_common *_ie)
{
	struct dect_ie_calling_party_number *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tNumber type: %s\n", dect_val2str(dect_number_types, buf, ie->type));
	sfmt_debug("\tNumbering Plan: %s\n", dect_val2str(dect_npis, buf, ie->npi));
	sfmt_debug("\tPresentation indicator: %s\n",
		   dect_val2str(dect_presentation_indicators, buf, ie->presentation));
	sfmt_debug("\tScreening indicator: %s\n",
		   dect_val2str(dect_screening_indicators, buf, ie->screening));
	sfmt_debug("\tAddress: %.*s\n", ie->len, ie->address);
}

static int dect_sfmt_parse_calling_party_number(const struct dect_handle *dh,
						struct dect_ie_common **ie,
						const struct dect_sfmt_ie *src)
{
	struct dect_ie_calling_party_number *dst = dect_ie_container(dst, *ie);
	unsigned int n = 2;

	dst->type         = (src->data[n] & 0x70) >> 4;
	dst->npi          = (src->data[n] & 0x0f);
	if (src->data[n] & DECT_OCTET_GROUP_END)
		goto group4;
	n++;
	dst->presentation = (src->data[n] & 0x3) >> 5;
	dst->screening    = (src->data[n] & 0x3);
	if (!(src->data[n] & DECT_OCTET_GROUP_END))
		return -1;
group4:
	n++;
	dst->len          = src->len - n;
	if (dst->len > array_size(dst->address))
		return -1;
	memcpy(dst->address, src->data + n, dst->len);
	return 0;
}

static int dect_sfmt_build_calling_party_number(struct dect_sfmt_ie *dst,
					        const struct dect_ie_common *ie)
{
	struct dect_ie_calling_party_number *src = dect_ie_container(src, ie);
	unsigned int n = 2;

	dst->data[n]  = src->type << 4;
	dst->data[n] |= src->npi;

	if (src->presentation == DECT_PRESENTATION_RESTRICTED ||
	    src->presentation == DECT_PRESENTATION_NOT_AVAILABLE) {
		dst->data[n] |= DECT_OCTET_GROUP_END;
		goto group4;
	}
	n++;
	dst->data[n]  = src->presentation << 5;
	dst->data[n] |= src->screening;
	dst->data[n] |= DECT_OCTET_GROUP_END;
group4:
	n++;
	memcpy(dst->data + n, src->address, src->len);
	dst->len = src->len + n;
	return 0;
}

static void dect_sfmt_dump_calling_party_name(const struct dect_ie_common *_ie)
{
	struct dect_ie_calling_party_name *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tPresentation indicator: %s\n",
		   dect_val2str(dect_presentation_indicators, buf, ie->presentation));
	sfmt_debug("\tUsed alphabet: %s\n",
		   dect_val2str(dect_alphabets, buf, ie->alphabet));
	sfmt_debug("\tScreening indicator: %s\n",
		   dect_val2str(dect_screening_indicators, buf, ie->screening));
	sfmt_debug("\tName: %.*s\n", ie->len, ie->name);
}

static int dect_sfmt_parse_calling_party_name(const struct dect_handle *dh,
					      struct dect_ie_common **ie,
					      const struct dect_sfmt_ie *src)
{
	struct dect_ie_calling_party_name *dst = dect_ie_container(dst, *ie);

	dst->presentation = (src->data[2] & 0x3) >> 5;
	dst->alphabet     = (src->data[2] & 0x7) >> 2;
	dst->screening    = (src->data[2] & 0x3);

	dst->len          = src->len - 3;
	if (dst->len > array_size(dst->name))
		return -1;
	memcpy(dst->name, src->data + 3, dst->len);
	return 0;
}

static int dect_sfmt_build_calling_party_name(struct dect_sfmt_ie *dst,
					      const struct dect_ie_common *ie)
{
	struct dect_ie_calling_party_name *src = dect_ie_container(src, ie);

	dst->data[2]  = src->presentation << 5;
	dst->data[2] |= src->alphabet << 2;
	dst->data[2] |= src->screening;
	memcpy(dst->data + 3, src->name, src->len);
	dst->len = src->len + 3;
	return 0;
}

static void dect_sfmt_dump_called_party_number(const struct dect_ie_common *_ie)
{
	struct dect_ie_called_party_number *ie = dect_ie_container(ie, _ie);
	char address[ie->len + 1];
	char buf[128];

	memcpy(address, ie->address, ie->len);
	address[ie->len] = '\0';

	sfmt_debug("\tNumber type: %s\n", dect_val2str(dect_number_types, buf, ie->type));
	sfmt_debug("\tNumbering Plan: %s\n", dect_val2str(dect_npis, buf, ie->npi));
	sfmt_debug("\tAddress: %s\n", address);
}

static int dect_sfmt_parse_called_party_number(const struct dect_handle *dh,
					       struct dect_ie_common **ie,
					       const struct dect_sfmt_ie *src)
{
	struct dect_ie_called_party_number *dst = dect_ie_container(dst, *ie);

	dst->type = (src->data[2] & 0x70) >> 4;
	dst->npi  = (src->data[2] & 0x0f);

	dst->len  = src->len - 3;
	if (dst->len > array_size(dst->address))
		return -1;
	memcpy(dst->address, src->data + 3, dst->len);
	return 0;
}

static int dect_sfmt_build_called_party_number(struct dect_sfmt_ie *dst,
					       const struct dect_ie_common *ie)
{
	struct dect_ie_called_party_number *src = dect_ie_container(src, ie);

	dst->data[2]  = src->type << 4;
	dst->data[2] |= src->npi;
	dst->data[2] |= DECT_OCTET_GROUP_END;
	memcpy(dst->data + 3, src->address, src->len);
	dst->len = src->len + 3;
	return 0;
}

static const struct dect_trans_tbl dect_lock_limits[] = {
	TRANS_TBL(DECT_LOCK_TEMPORARY_USER_LIMIT_1,	"temporary user limit 1"),
	TRANS_TBL(DECT_LOCK_NO_LIMITS,			"no limits"),
	TRANS_TBL(DECT_LOCK_TEMPORARY_USER_LIMIT_2,	"temporary user limit 2"),
};

static const struct dect_trans_tbl dect_time_limits[] = {
	TRANS_TBL(DECT_TIME_LIMIT_ERASE,		"erase"),
	TRANS_TBL(DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1,	"defined time limit 1"),
	TRANS_TBL(DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_2,	"defined time limit 2"),
	TRANS_TBL(DECT_TIME_LIMIT_STANDARD_TIME_LIMIT,	"standard time limit"),
	TRANS_TBL(DECT_TIME_LIMIT_INFINITE,		"infinite"),
};

static void dect_sfmt_dump_duration(const struct dect_ie_common *_ie)
{
	struct dect_ie_duration *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tlock: %s\n", dect_val2str(dect_lock_limits, buf, ie->lock));
	sfmt_debug("\ttime: %s\n", dect_val2str(dect_time_limits, buf, ie->time));
	sfmt_debug("\tduration: %u\n", ie->duration);
}

static int dect_sfmt_parse_duration(const struct dect_handle *dh,
				    struct dect_ie_common **ie,
				    const struct dect_sfmt_ie *src)
{
	struct dect_ie_duration *dst = dect_ie_container(dst, *ie);

	dst->lock = src->data[2] & 0x70;
	dst->time = src->data[2] & 0x0f;
	if (!(src->data[2] & DECT_OCTET_GROUP_END))
		dst->duration = src->data[3];
	return 0;
}

static int dect_sfmt_build_duration(struct dect_sfmt_ie *dst,
				    const struct dect_ie_common *ie)
{
	struct dect_ie_duration *src = dect_ie_container(src, ie);

	dst->len = 3;
	dst->data[2] = (src->lock << 4) | src->time;
	if (src->time != DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1 &&
	    src->time != DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_2)
		dst->data[2] |= DECT_OCTET_GROUP_END;
	else {
		dst->data[3] = src->duration;
		dst->len++;
	}
	return 0;
}

static const struct dect_trans_tbl dect_iwu_to_iwu_sr[] = {
	TRANS_TBL(false,							"Rejection of message"),
	TRANS_TBL(true,								"Transmission of message"),
};

static const struct dect_trans_tbl dect_iwu_to_iwu_pds[] = {
	TRANS_TBL(DECT_IWU_TO_IWU_PD_USER_SPECIFIC,				"User Specific"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_OSI_HIGHER_LAYER,				"OSI high layer protocols"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_ITU_T_X263,				"ITU-T Recommendation X.263"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_LIST_ACCESS,				"List Access"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_IA5_CHARACTERS,				"IA5 characters"),
	TRANS_TBL(DECt_IWU_TO_IWU_PD_LDS_SUOTA,					"Light data service, Software Upgrade Over The Air (SUOTA)"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_ITU_T_V120,				"ITU-T Recommendation V.120 Rate adaption"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_ITU_T_Q931_MESSAGE,			"ITU-T Recommendation Q.931 [i.15] (I.451), message"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_ITU_T_Q931_IE,				"ITU-T Recommendation Q.931 [i.15] (I.451), information element(s)"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_ITU_T_Q931_PARTIAL_MESSAGE,		"ITU-T Recommendation Q.931 [i.15] (I.451), partial message"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_GSM_MESSAGE,				"GSM, message"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_GSM_IE,					"GSM, information element(s)"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_UMTS_GPRS_IE,				"UMTS/GPRS information element(s)"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_UMTS_GPRS_MESSAGE,				"UMTS/GPRS, messages"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_LRMS,					"LRMS"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_RLL_ACCESS_PROFILE,			"RLL access profile"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_WRS,					"WRS"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_DECT_ISDN_C_PLANE_SPECIFIC,		"DECT/ISDN Intermediate System C-plane specific"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_DECT_ISDN_U_PLANE_SPECIFIC,		"DECT/ISDN Intermediate System U-plane specific"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_DECT_ISDN_OPERATION_AND_MAINTENANCE,	"DECT/ISDN Intermediate System Operation and Maintenance"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_TERMINAL_DATA,				"Terminal Data"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_DECT_IP_NETWORK_ACCESS_SPECIFIC,		"DECT access to IP Networks specific"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_MPEG4_ER_AAL_LD_CONFIGURATION,		"MPEG-4 ER AAC-LD Configuration Description"),
	TRANS_TBL(DECT_IWU_TO_IWU_PD_UNKNOWN,					"Unknown"),
};

static void dect_sfmt_dump_iwu_to_iwu(const struct dect_ie_common *_ie)
{
	struct dect_ie_iwu_to_iwu *ie = dect_ie_container(ie, _ie);
	char buf[128];

	sfmt_debug("\tSend/Reject (S/R) bit: %s\n",
		   dect_val2str(dect_iwu_to_iwu_sr, buf, ie->sr));
	sfmt_debug("\tProtocol Discriminator: %s\n",
		   dect_val2str(dect_iwu_to_iwu_pds, buf, ie->pd));
	sfmt_debug("\tContent length: %u\n", ie->len);
	dect_hexdump(DECT_DEBUG_SFMT, "\tInformation", ie->data, ie->len);
}

static int dect_sfmt_parse_iwu_to_iwu(const struct dect_handle *dh,
				      struct dect_ie_common **ie,
				      const struct dect_sfmt_ie *src)
{
	struct dect_ie_iwu_to_iwu *dst = dect_ie_container(dst, *ie);

	dst->sr  = src->data[2] & 0x40;
	dst->pd  = src->data[2] & 0x3f;
	if (!(src->data[2] & DECT_OCTET_GROUP_END))
		return -1;

	dst->len = src->len - 3;
	if (dst->len > array_size(dst->data))
		return -1;
	memcpy(dst->data, src->data + 3, dst->len);
	return 0;
}

static int dect_sfmt_build_iwu_to_iwu(struct dect_sfmt_ie *dst,
				      const struct dect_ie_common *ie)
{
	struct dect_ie_iwu_to_iwu *src = dect_ie_container(src, ie);

	dst->data[2]  = src->sr ? 0x40 : 0x0;
	dst->data[2] |= src->pd;
	dst->data[2] |= DECT_OCTET_GROUP_END;
	memcpy(dst->data + 3, src->data, src->len);
	dst->len = src->len + 3;
	return 0;
}

static void dect_sfmt_dump_escape_to_proprietary(const struct dect_ie_common *_ie)
{
	struct dect_ie_escape_to_proprietary *ie = dect_ie_container(ie, _ie);

	sfmt_debug("\tEMC: %x\n", ie->emc);
	dect_hexdump(DECT_DEBUG_SFMT, "\tContent", ie->content, ie->len);
}

static int dect_sfmt_build_escape_to_proprietary(struct dect_sfmt_ie *dst,
						 const struct dect_ie_common *ie)
{
	struct dect_ie_escape_to_proprietary *src = dect_ie_container(src, ie);

	dst->data[2]  = DECT_ESC_TO_PROPRIETARY_IE_DESC_EMC;
	dst->data[2] |= DECT_OCTET_GROUP_END;
	*(uint16_t *)&dst->data[3] = __cpu_to_be16(src->emc);
	memcpy(dst->data + 5, src->content, src->len);
	dst->len = 5 + src->len;
	return 0;
}

static int dect_sfmt_parse_escape_to_proprietary(const struct dect_handle *dh,
						 struct dect_ie_common **ie,
						 const struct dect_sfmt_ie *src)
{
	struct dect_ie_escape_to_proprietary *dst = dect_ie_container(dst, *ie);
	uint8_t dtype;

	dtype = (src->data[2] & DECT_ESC_TO_PROPRIETARY_IE_DESC_TYPE_MASK);
	if (dtype != DECT_ESC_TO_PROPRIETARY_IE_DESC_EMC)
		return -1;
	dst->emc = __be16_to_cpu(*(__be16 *)&src->data[3]);

	dst->len = src->len - 5;
	if (dst->len > array_size(dst->content))
		return -1;
	memcpy(dst->content, src->data + 5, dst->len);
	return 0;
}

static const struct dect_trans_tbl dect_negotiation_indicators[] = {
	TRANS_TBL(DECT_NEGOTIATION_NOT_POSSIBLE,	"negotiation not possible"),
	TRANS_TBL(DECT_NEGOTIATION_CODEC,		"codec negotiation"),
};

static const struct dect_trans_tbl dect_codecs[] = {
	TRANS_TBL(DECT_CODEC_USER_SPECIFIC_32KBIT,	"user specific (32kbit)"),
	TRANS_TBL(DECT_CODEC_G726_32KBIT,		"G.726 (32kbit)"),
	TRANS_TBL(DECT_CODEC_G722_64KBIT,		"G.722 (64kbit)"),
	TRANS_TBL(DECT_CODEC_G711_ALAW_64KBIT,		"G.711 A-law (64kbit)"),
	TRANS_TBL(DECT_CODEC_G711_ULAW_64KBIT,		"G.711 U-law (64kbit)"),
	TRANS_TBL(DECT_CODEC_G729_1_32KBIT,		"G.729.1 (32kbit)"),
	TRANS_TBL(DECT_CODEC_MPEG4_ER_AAC_LD_32KBIT,	"MPEG4 ER AAC-LD (32kbit)"),
	TRANS_TBL(DECT_CODEC_MPEG4_ER_AAC_LD_64KBIT,	"MPEG4 ER AAC-LD (64kbit)"),
	TRANS_TBL(DECT_CODEC_USER_SPECIFIC_64KBIT,	"User specific (64kbit)"),
};

static const struct dect_trans_tbl dect_mac_dlc_services[] = {
	TRANS_TBL(DECT_MAC_DLC_SERVICE_LU1_INA,		"DLC service: LU1, MAC service: I_NA"),
	TRANS_TBL(DECT_MAC_DLC_SERVICE_LU1_INB,		"DLC service: LU1, MAC service: I_NB"),
	TRANS_TBL(DECT_MAC_DLC_SERVICE_LU1_IPM,		"DLC service: LU1, MAC service: I_PM"),
	TRANS_TBL(DECT_MAC_DLC_SERVICE_LU1_IPQ,		"DLC service: LU1, MAC service: I_PQ"),
	TRANS_TBL(DECT_MAC_DLC_SERVICE_LU7_INB,		"DLC service: LU7, MAC service: I_N"),
	TRANS_TBL(DECT_MAC_DLC_SERVICE_LU12_INB,	"DLC service: LU12, MAC service: I_NB"),
};

static const struct dect_trans_tbl dect_slot_sizes[] = {
	TRANS_TBL(DECT_HALF_SLOT_SIZE,			"half slot"),
	TRANS_TBL(DECT_LONG_SLOT_640_SIZE,		"long slot j=640"),
	TRANS_TBL(DECT_LONG_SLOT_672_SIZE,		"long slot j=672"),
	TRANS_TBL(DECT_FULL_SLOT_SIZE,			"full slot"),
	TRANS_TBL(DECT_DOUBLE_SLOT_SIZE,		"double slot"),
};

static const struct dect_trans_tbl dect_cplane_routing[] = {
	TRANS_TBL(DECT_CPLANE_CS_ONLY,			"C_S only"),
	TRANS_TBL(DECT_CPLANE_CS_PREFERRED,		"C_S preferred, C_F accepted"),
	TRANS_TBL(DECT_CPLANE_CF_PREFERRED,		"C_F preferred, C_S accepted"),
	TRANS_TBL(DECT_CPLANE_CF_ONLY,			"C_F only"),
};

static void dect_sfmt_dump_codec_list(const struct dect_ie_common *_ie)
{
	struct dect_ie_codec_list *ie = dect_ie_container(ie, _ie);
	unsigned int i;
	char buf[128];

	sfmt_debug("\tNegotiation Indicator: %s\n",
		   dect_val2str(dect_negotiation_indicators, buf, ie->negotiation));

	for (i = 0; i < ie->num; i++) {
		sfmt_debug("\tCodec %u:\n", i + 1);
		sfmt_debug("\t Codec: %s\n",
			   dect_val2str(dect_codecs, buf, ie->entry[i].codec));
		sfmt_debug("\t MAC/DLC Service: %s\n",
			   dect_val2str(dect_mac_dlc_services, buf, ie->entry[i].service));
		sfmt_debug("\t Slot size: %s\n",
			   dect_val2str(dect_slot_sizes, buf, ie->entry[i].slot));
		sfmt_debug("\t C-Plane routing: %s\n",
			   dect_val2str(dect_cplane_routing, buf, ie->entry[i].cplane));
	}
}

static int dect_sfmt_parse_codec_list(const struct dect_handle *dh,
				      struct dect_ie_common **ie,
				      const struct dect_sfmt_ie *src)
{
	struct dect_ie_codec_list *dst = dect_ie_container(dst, *ie);
	unsigned int n = 2;

	dst->negotiation = (src->data[n] & ~DECT_OCTET_GROUP_END) >> 4;
	n++;

	while (src->len - n >= 3) {
		dst->entry[dst->num].codec = src->data[n];
		n++;
		dst->entry[dst->num].service = src->data[n] & 0x0f;
		n++;
		dst->entry[dst->num].cplane = (src->data[n] & 0x70) >> 4;
		dst->entry[dst->num].slot = src->data[n] & 0x0f;
		n++;

		dst->num++;
		if (dst->num == array_size(dst->entry))
			break;
	}
	return 0;
}

static int dect_sfmt_build_codec_list(struct dect_sfmt_ie *dst,
				      const struct dect_ie_common *ie)
{
	struct dect_ie_codec_list *src = dect_ie_container(src, ie);
	unsigned int n = 2, i;

	dst->data[n] = (src->negotiation << 4) | DECT_OCTET_GROUP_END;
	n++;

	for (i = 0; i < src->num; i++) {
		dst->data[n]  = src->entry[i].codec;
		n++;
		dst->data[n]  = src->entry[i].service;
		n++;
		dst->data[n]  = src->entry[i].cplane;
		dst->data[n] |= src->entry[i].slot;
		n++;
	}
	dst->data[n - 1] |= DECT_OCTET_GROUP_END;

	dst->len = n;
	return 0;
}

static const struct dect_trans_tbl dect_event_types[] = {
	TRANS_TBL(DECT_EVENT_MESSAGE_WAITING,		"Message waiting"),
	TRANS_TBL(DECT_EVENT_MISSED_CALL,		"Missed call"),
	TRANS_TBL(DECT_EVENT_WEB_CONTENT,		"Web content"),
	TRANS_TBL(DECT_EVENT_LIST_CHANGE_INDICATION,	"List change indication"),
};

static void dect_sfmt_dump_events_notification(const struct dect_ie_common *_ie)
{
	struct dect_ie_events_notification *ie = dect_ie_container(ie, _ie);
	unsigned int i;
	char buf[128];

	for (i = 0; i < ie->num; i++) {
		sfmt_debug("\tEvent %u:\n", i + 1);
		sfmt_debug("\t Event type: %s\n",
			   dect_val2str(dect_event_types, buf, ie->events[i].type));
		sfmt_debug("\t Event subtype: %u\n", ie->events[i].subtype);
		sfmt_debug("\t Event multiplicity: %u\n", ie->events[i].multiplicity);
	}
}

static int dect_sfmt_build_events_notification(struct dect_sfmt_ie *dst,
					       const struct dect_ie_common *ie)
{
	struct dect_ie_events_notification *src = dect_ie_container(src, ie);
	unsigned int n = 2, i;

	for (i = 0; i < src->num; i++) {
		dst->data[n++] = src->events[i].type;
		dst->data[n++] = src->events[i].subtype | DECT_OCTET_GROUP_END;
		dst->data[n++] = src->events[i].multiplicity | DECT_OCTET_GROUP_END;
	}

	dst->len = n;
	return 0;
}

static const struct dect_ie_handler {
	const char	*name;
	size_t		size;
	int		(*parse)(const struct dect_handle *dh,
				 struct dect_ie_common **dst,
				 const struct dect_sfmt_ie *ie);
	int		(*build)(struct dect_sfmt_ie *dst,
				 const struct dect_ie_common *ie);
	void		(*dump)(const struct dect_ie_common *ie);
} dect_ie_handlers[256] = {
	[DECT_IE_REPEAT_INDICATOR]		= {
		.name	= "REPEAT-INDICATOR",
		.parse	= dect_sfmt_parse_repeat_indicator,
		.build	= dect_sfmt_build_repeat_indicator,
		.dump	= dect_sfmt_dump_repeat_indicator,
	},
	[DECT_IE_SENDING_COMPLETE]		= {
		.name	= "SENDING-COMPLETE",
		.size	= sizeof(struct dect_ie_sending_complete),
		.parse	= dect_sfmt_parse_empty_single_octet,
		.build	= dect_sfmt_build_empty_single_octet,
	},
	[DECT_IE_DELIMITER_REQUEST]		= {
		.name	= "DELIMITER-REQUEST",
		.size	= sizeof(struct dect_ie_delimiter_request),
		.parse	= dect_sfmt_parse_empty_single_octet,
		.build	= dect_sfmt_build_empty_single_octet,
	},
	[DECT_IE_USE_TPUI]			= {
		.name	= "USE-TPUI",
		.size	= sizeof(struct dect_ie_use_tpui),
		.parse	= dect_sfmt_parse_empty_single_octet,
		.build	= dect_sfmt_build_empty_single_octet,
	},
	[DECT_IE_BASIC_SERVICE]			= {
		.name	= "BASIC-SERVICE",
		.size	= sizeof(struct dect_ie_basic_service),
		.parse	= dect_sfmt_parse_basic_service,
		.build	= dect_sfmt_build_basic_service,
		.dump	= dect_sfmt_dump_basic_service,
	},
	[DECT_IE_RELEASE_REASON]		= {
		.name	= "RELEASE-REASON",
		.size	= sizeof(struct dect_ie_release_reason),
		.parse	= dect_sfmt_parse_release_reason,
		.build	= dect_sfmt_build_release_reason,
		.dump	= dect_sfmt_dump_release_reason,
	},
	[DECT_IE_SIGNAL]			= {
		.name	= "SIGNAL",
		.size	= sizeof(struct dect_ie_signal),
		.parse	= dect_sfmt_parse_signal,
		.build	= dect_sfmt_build_signal,
		.dump	= dect_sfmt_dump_signal,
	},
	[DECT_IE_TIMER_RESTART]			= {
		.name	= "TIMER-RESTART",
		.size	= sizeof(struct dect_ie_timer_restart),
		.parse	= dect_sfmt_parse_timer_restart,
	},
	[DECT_IE_TEST_HOOK_CONTROL]		= {
		.name	= "TEST-HOOK-CONTROL",
	},
	[DECT_IE_SINGLE_DISPLAY]		= {
		.name	= "SINGLE-DISPLAY",
		.size	= sizeof(struct dect_ie_display),
		.parse	= dect_sfmt_parse_single_display,
		.build	= dect_sfmt_build_single_display,
		.dump	= dect_sfmt_dump_display,
	},
	[DECT_IE_SINGLE_KEYPAD]			= {
		.name	= "SINGLE-KEYPAD",
		.size	= sizeof(struct dect_ie_keypad),
		.parse	= dect_sfmt_parse_single_keypad,
		.build	= dect_sfmt_build_single_keypad,
		.dump	= dect_sfmt_dump_keypad,
	},
	[DECT_IE_INFO_TYPE]			= {
		.name	= "INFO-TYPE",
		.size	= sizeof(struct dect_ie_info_type),
		.parse	= dect_sfmt_parse_info_type,
		.build	= dect_sfmt_build_info_type,
		.dump	= dect_sfmt_dump_info_type,
	},
	[DECT_IE_IDENTITY_TYPE]			= {
		.name	= "IDENTITY-TYPE",
		.size	= sizeof(struct dect_ie_identity_type),
		.parse	= dect_sfmt_parse_identity_type,
		.build	= dect_sfmt_build_identity_type,
		.dump	= dect_sfmt_dump_identity_type,
	},
	[DECT_IE_PORTABLE_IDENTITY]		= {
		.name	= "PORTABLE-IDENTITY",
		.size	= sizeof(struct dect_ie_portable_identity),
		.parse	= dect_sfmt_parse_portable_identity,
		.build	= dect_sfmt_build_portable_identity,
		.dump	= dect_sfmt_dump_portable_identity,
	},
	[DECT_IE_FIXED_IDENTITY]		= {
		.name	= "FIXED-IDENTITY",
		.size	= sizeof(struct dect_ie_fixed_identity),
		.parse	= dect_sfmt_parse_fixed_identity,
		.build	= dect_sfmt_build_fixed_identity,
		.dump	= dect_sfmt_dump_fixed_identity,
	},
	[DECT_IE_LOCATION_AREA]			= {
		.name	= "LOCATION-AREA",
		.size	= sizeof(struct dect_ie_location_area),
		.parse	= dect_sfmt_parse_location_area,
		.build	= dect_sfmt_build_location_area,
		.dump	= dect_sfmt_dump_location_area,
	},
	[DECT_IE_NWK_ASSIGNED_IDENTITY]		= {
		.name	= "NWK-ASSIGNED-IDENTITY",
		.size	= sizeof(struct dect_ie_nwk_assigned_identity),
	},
	[DECT_IE_ALLOCATION_TYPE]		= {
		.name	= "ALLOCATION-TYPE",
		.size	= sizeof(struct dect_ie_allocation_type),
		.parse	= dect_sfmt_parse_allocation_type,
		.build	= dect_sfmt_build_allocation_type,
		.dump	= dect_sfmt_dump_allocation_type,
	},
	[DECT_IE_AUTH_TYPE]			= {
		.name	= "AUTH-TYPE",
		.size	= sizeof(struct dect_ie_auth_type),
		.parse	= dect_sfmt_parse_auth_type,
		.build	= dect_sfmt_build_auth_type,
		.dump	= dect_sfmt_dump_auth_type,
	},
	[DECT_IE_RAND]				= {
		.name	= "RAND",
		.size	= sizeof(struct dect_ie_auth_value),
		.parse	= dect_sfmt_parse_auth_value,
		.build	= dect_sfmt_build_auth_value,
		.dump	= dect_sfmt_dump_auth_value,
	},
	[DECT_IE_RES]				= {
		.name	= "RES",
		.size	= sizeof(struct dect_ie_auth_res),
		.parse	= dect_sfmt_parse_auth_res,
		.build	= dect_sfmt_build_auth_res,
		.dump	= dect_sfmt_dump_auth_res,
	},
	[DECT_IE_RS]				= {
		.name	= "RS",
		.size	= sizeof(struct dect_ie_auth_value),
		.parse	= dect_sfmt_parse_auth_value,
		.build	= dect_sfmt_build_auth_value,
		.dump	= dect_sfmt_dump_auth_value,
	},
	[DECT_IE_IWU_ATTRIBUTES]		= {
		.name	= "IWU-ATTRIBUTES",
		.size	= sizeof(struct dect_ie_iwu_attributes),
	},
	[DECT_IE_CALL_ATTRIBUTES]		= {
		.name	= "CALL-ATTRIBUTES",
		.size	= sizeof(struct dect_ie_call_attributes),
	},
	[DECT_IE_SERVICE_CHANGE_INFO]		= {
		.name	= "SERVICE-CHANGE-INFO",
		.size	= sizeof(struct dect_ie_service_change_info),
		.parse	= dect_sfmt_parse_service_change_info,
		.dump	= dect_sfmt_dump_service_change_info,
	},
	[DECT_IE_CONNECTION_ATTRIBUTES]		= {
		.name	= "CONNECTION-ATTRIBUTES",
		.size	= sizeof(struct dect_ie_connection_attributes),
	},
	[DECT_IE_CIPHER_INFO]			= {
		.name	= "CIPHER-INFO",
		.size	= sizeof(struct dect_ie_cipher_info),
		.parse	= dect_sfmt_parse_cipher_info,
		.build	= dect_sfmt_build_cipher_info,
		.dump	= dect_sfmt_dump_cipher_info,
	},
	[DECT_IE_CALL_IDENTITY]			= {
		.name	= "CALL-IDENTITY",
		.size	= sizeof(struct dect_ie_call_identity),
	},
	[DECT_IE_CONNECTION_IDENTITY]		= {
		.name	= "CONNECTION-IDENTITY",
		.size	= sizeof(struct dect_ie_connection_identity),
	},
	[DECT_IE_FACILITY]			= {
		.name	= "FACILITY",
		.size	= sizeof(struct dect_ie_facility),
		.parse	= dect_sfmt_parse_facility,
		.dump	= dect_sfmt_dump_facility,
	},
	[DECT_IE_PROGRESS_INDICATOR]		= {
		.name	= "PROGRESS-INDICATOR",
		.size	= sizeof(struct dect_ie_progress_indicator),
		.parse	= dect_sfmt_parse_progress_indicator,
		.build	= dect_sfmt_build_progress_indicator,
		.dump	= dect_sfmt_dump_progress_indicator,
	},
	[DECT_IE_MMS_GENERIC_HEADER]		= {
		.name	= "MMS-GENERIC-HEADER",
		.size	= sizeof(struct dect_ie_mms_generic_header),
	},
	[DECT_IE_MMS_OBJECT_HEADER]		= {
		.name	= "MMS-OBJECT-HEADER",
		.size	= sizeof(struct dect_ie_mms_object_header),
	},
	[DECT_IE_MMS_EXTENDED_HEADER]		= {
		.name	= "MMS-EXTENDED-HEADER",
		.size	= sizeof(struct dect_ie_mms_extended_header),
	},
	[DECT_IE_TIME_DATE]			= {
		.name	= "TIME-DATE",
		.size	= sizeof(struct dect_ie_time_date),
		.parse	= dect_sfmt_parse_time_date,
		.build	= dect_sfmt_build_time_date,
		.dump	= dect_sfmt_dump_time_date,
	},
	[DECT_IE_MULTI_DISPLAY]			= {
		.name	= "MULTI-DISPLAY",
		.size	= sizeof(struct dect_ie_display),
		.parse	= dect_sfmt_parse_multi_display,
		.build	= dect_sfmt_build_multi_display,
		.dump	= dect_sfmt_dump_display,
	},
	[DECT_IE_MULTI_KEYPAD]			= {
		.name	= "MULTI-KEYPAD",
		.size	= sizeof(struct dect_ie_keypad),
		.parse	= dect_sfmt_parse_multi_keypad,
		.build	= dect_sfmt_build_multi_keypad,
		.dump	= dect_sfmt_dump_keypad,
	},
	[DECT_IE_FEATURE_ACTIVATE]		= {
		.name	= "FEATURE-ACTIVATE",
		.size	= sizeof(struct dect_ie_feature_activate),
		.parse	= dect_sfmt_parse_feature_activate,
		.build	= dect_sfmt_build_feature_activate,
		.dump	= dect_sfmt_dump_feature_activate,
	},
	[DECT_IE_FEATURE_INDICATE]		= {
		.name	= "FEATURE-INDICATE",
		.size	= sizeof(struct dect_ie_feature_indicate),
		.parse	= dect_sfmt_parse_feature_indicate,
		.dump	= dect_sfmt_dump_feature_indicate,
	},
	[DECT_IE_NETWORK_PARAMETER]		= {
		.name	= "NETWORK-PARAMETER",
		.size	= sizeof(struct dect_ie_network_parameter),
		.build	= dect_sfmt_build_network_parameter,
		.parse	= dect_sfmt_parse_network_parameter,
		.dump	= dect_sfmt_dump_network_parameter,
	},
	[DECT_IE_EXT_HO_INDICATOR]		= {
		.name	= "EXT-H/O-INDICATOR",
		.size	= sizeof(struct dect_ie_ext_ho_indicator),
	},
	[DECT_IE_ZAP_FIELD]			= {
		.name	= "ZAP-FIELD",
		.size	= sizeof(struct dect_ie_zap_field),
	},
	[DECT_IE_SERVICE_CLASS]			= {
		.name	= "SERVICE-CLASS",
		.size	= sizeof(struct dect_ie_service_class),
	},
	[DECT_IE_KEY]				= {
		.name	= "KEY",
		.size	= sizeof(struct dect_ie_key),
	},
	[DECT_IE_REJECT_REASON]			= {
		.name	= "REJECT-REASON",
		.size	= sizeof(struct dect_ie_reject_reason),
		.parse	= dect_sfmt_parse_reject_reason,
		.build	= dect_sfmt_build_reject_reason,
		.dump	= dect_sfmt_dump_reject_reason,
	},
	[DECT_IE_SETUP_CAPABILITY]		= {
		.name	= "SETUP-CAPABILITY",
		.size	= sizeof(struct dect_ie_setup_capability),
		.parse	= dect_sfmt_parse_setup_capability,
		.build	= dect_sfmt_build_setup_capability,
	},
	[DECT_IE_TERMINAL_CAPABILITY]		= {
		.name	= "TERMINAL-CAPABILITY",
		.size	= sizeof(struct dect_ie_terminal_capability),
		.parse	= dect_sfmt_parse_terminal_capability,
		.build	= dect_sfmt_build_terminal_capability,
		.dump	= dect_sfmt_dump_terminal_capability,
	},
	[DECT_IE_END_TO_END_COMPATIBILITY]	= {
		.name	= "END-TO-END-COMPATIBILITY",
		.size	= sizeof(struct dect_ie_end_to_end_compatibility),
	},
	[DECT_IE_RATE_PARAMETERS]		= {
		.name	= "RATE-PARAMETERS",
		.size	= sizeof(struct dect_ie_rate_parameters),
	},
	[DECT_IE_TRANSIT_DELAY]			= {
		.name	= "TRANSIT-DELAY",
		.size	= sizeof(struct dect_ie_transit_delay),
	},
	[DECT_IE_WINDOW_SIZE]			= {
		.name	= "WINDOW-SIZE",
		.size	= sizeof(struct dect_ie_window_size),
	},
	[DECT_IE_CALLING_PARTY_NUMBER]		= {
		.name	= "CALLING-PARTY-NUMBER",
		.size	= sizeof(struct dect_ie_calling_party_number),
		.parse	= dect_sfmt_parse_calling_party_number,
		.build	= dect_sfmt_build_calling_party_number,
		.dump	= dect_sfmt_dump_calling_party_number,
	},
	[DECT_IE_CALLING_PARTY_NAME]		= {
		.name	= "CALLING-PARTY-NAME",
		.size	= sizeof(struct dect_ie_calling_party_name),
		.parse	= dect_sfmt_parse_calling_party_name,
		.build	= dect_sfmt_build_calling_party_name,
		.dump	= dect_sfmt_dump_calling_party_name,
	},
	[DECT_IE_CALLED_PARTY_NUMBER]		= {
		.name	= "CALLED-PARTY-NUMBER",
		.size	= sizeof(struct dect_ie_called_party_number),
		.parse	= dect_sfmt_parse_called_party_number,
		.build	= dect_sfmt_build_called_party_number,
		.dump	= dect_sfmt_dump_called_party_number,
	},
	[DECT_IE_CALLED_PARTY_SUBADDR]		= {
		.name	= "CALLED-PARTY-SUBADDRESS",
		.size	= sizeof(struct dect_ie_called_party_subaddress),
	},
	[DECT_IE_DURATION]			= {
		.name	= "DURATION",
		.size	= sizeof(struct dect_ie_duration),
		.parse	= dect_sfmt_parse_duration,
		.build	= dect_sfmt_build_duration,
		.dump	= dect_sfmt_dump_duration,
	},
	[DECT_IE_SEGMENTED_INFO]		= {
		.name	= "SEGMENTED-INFO",
		.size	= sizeof(struct dect_ie_segmented_info),
	},
	[DECT_IE_ALPHANUMERIC]			= {
		.name	= "ALPHANUMERIC",
		.size	= sizeof(struct dect_ie_alphanumeric),
	},
	[DECT_IE_IWU_TO_IWU]			= {
		.name	= "IWU-TO-IWU",
		.size	= sizeof(struct dect_ie_iwu_to_iwu),
		.parse	= dect_sfmt_parse_iwu_to_iwu,
		.build	= dect_sfmt_build_iwu_to_iwu,
		.dump	= dect_sfmt_dump_iwu_to_iwu,
	},
	[DECT_IE_MODEL_IDENTIFIER]		= {
		.name	= "MODEL-IDENTIFIER",
		.size	= sizeof(struct dect_ie_model_identifier),
	},
	[DECT_IE_IWU_PACKET]			= {
		.name	= "IWU-PACKET",
		.size	= sizeof(struct dect_ie_iwu_packet),
	},
	[DECT_IE_ESCAPE_TO_PROPRIETARY]		= {
		.name	= "ESCAPE-TO-PROPRIETARY",
		.size	= sizeof(struct dect_ie_escape_to_proprietary),
		.parse	= dect_sfmt_parse_escape_to_proprietary,
		.build	= dect_sfmt_build_escape_to_proprietary,
		.dump	= dect_sfmt_dump_escape_to_proprietary,
	},
	[DECT_IE_CODEC_LIST]			= {
		.name	= "CODEC-LIST",
		.size	= sizeof(struct dect_ie_codec_list),
		.parse	= dect_sfmt_parse_codec_list,
		.build	= dect_sfmt_build_codec_list,
		.dump	= dect_sfmt_dump_codec_list,
	},
	[DECT_IE_EVENTS_NOTIFICATION]		= {
		.name	= "EVENTS-NOTIFICATION",
		.size	= sizeof(struct dect_ie_events_notification),
		.build	= dect_sfmt_build_events_notification,
		.dump	= dect_sfmt_dump_events_notification,
	},
	[DECT_IE_CALL_INFORMATION]		= {
		.name	= "CALL-INFORMATION",
		.size	= sizeof(struct dect_ie_call_information),
	},
	[DECT_IE_ESCAPE_FOR_EXTENSION]		= {
		.name	= "ESCAPE-FOR-EXTENSION",
	},
};

static enum dect_sfmt_ie_status dect_rx_status(const struct dect_handle *dh,
					       const struct dect_sfmt_ie_desc *desc)
{
	if (dh->mode == DECT_MODE_FP)
		return desc->pp_fp;
	else
		return desc->fp_pp;
}

static enum dect_sfmt_ie_status dect_tx_status(const struct dect_handle *dh,
					       const struct dect_sfmt_ie_desc *desc)
{
	if (dh->mode == DECT_MODE_FP)
		return desc->fp_pp;
	else
		return desc->pp_fp;
}

static struct dect_ie_common **
dect_next_ie(const struct dect_sfmt_ie_desc *desc, struct dect_ie_common **ie)
{
	if (desc->type == DECT_IE_REPEAT_INDICATOR)
		return ((void *)ie) + sizeof(struct dect_ie_list);
	else if (!(desc->flags & DECT_SFMT_IE_REPEAT))
		return ie + 1;
	else
		return ie;
}

static void dect_msg_ie_init(const struct dect_sfmt_ie_desc *desc,
			     struct dect_ie_common **ie)
{
	struct dect_ie_list *iel;

	if (desc->flags & DECT_SFMT_IE_END)
		return;

	if (desc->type == DECT_IE_REPEAT_INDICATOR) {
		iel = dect_ie_container(iel, (struct dect_ie_common *)ie);
		dect_ie_list_init(iel);
	} else if (!(desc->flags & DECT_SFMT_IE_REPEAT))
		*ie = NULL;
	else
		return;
#if 0
	sfmt_debug("init message IE %p: <%s>\n",
		 ie, dect_ie_handlers[desc->type].name);
#endif
}

/**
 * Parse a S-Format encoded Information Element header
 *
 * @param ie	result pointer to the Information Element
 * @param mb	message buffer
 *
 * Parse a S-Format encoded Information Element header and return the parsed
 * information in the ie structure.
 *
 * @return 0 on success or -1 if a parsing error occured.
 */
enum dect_sfmt_error
dect_parse_sfmt_ie_header(struct dect_sfmt_ie *ie,
			  const struct dect_msg_buf *mb)
{
	uint8_t val;

	if (mb->len < 1)
		return -1;

	ie->id = mb->data[0] & DECT_SFMT_IE_FIXED_LEN;
	if (ie->id & DECT_SFMT_IE_FIXED_LEN) {
		ie->id |= (mb->data[0] & DECT_SFMT_IE_FIXED_ID_MASK);
		val     = (mb->data[0] & DECT_SFMT_IE_FIXED_VAL_MASK);
		if (ie->id != DECT_IE_DOUBLE_OCTET_ELEMENT) {
			ie->len = 1;
			if (ie->id == DECT_IE_EXT_PREFIX)
				ie->id |= val;
		} else {
			if (mb->len < 2)
				return -1;
			ie->id |= val;
			ie->len = 2;
		}
	} else {
		if (mb->len < 2U || mb->len < 2U + mb->data[1])
			return -1;
		ie->id  = mb->data[0];
		ie->len = mb->data[1] + 2;
	}
	ie->data = mb->data;

//	sfmt_debug("found IE: <<%s>> (%x) len: %u\n", dect_ie_handlers[ie->id].name,
//		   ie->id, ie->len);
	return 0;
}
EXPORT_SYMBOL(dect_parse_sfmt_ie_header);

static int dect_build_sfmt_ie_header(struct dect_sfmt_ie *dst, uint8_t id)
{
	if (id & DECT_SFMT_IE_FIXED_LEN) {
		dst->data[0] |= id;
		if ((id & DECT_SFMT_IE_FIXED_ID_MASK) !=
		    (DECT_IE_DOUBLE_OCTET_ELEMENT & DECT_SFMT_IE_FIXED_ID_MASK))
			dst->len = 1;
		else
			dst->len = 2;
	} else {
		if (dst->len == 2)
			dst->len = 0;
		else {
			dect_assert(dst->len > 2);
			dst->data[1] = dst->len - 2;
			dst->data[0] = id;
		}
	}
	return 0;
}

/**
 * Parse a S-Format encoded Information Element
 *
 * @param dh		libdect DECT handle
 * @param type		IE type
 * @param dst		result pointer to the allocated information element
 * @param ie		information element
 *
 * Parse a S-Format encoded Information Element and return an allocated IE
 * structure.
 *
 * @return #DECT_SFMT_OK on success or one of the @ref dect_sfmt_error
 * "S-Format error codes" on error. On success the dst parameter is set to
 * point to the allocated information element structure.
 */
enum dect_sfmt_error
dect_parse_sfmt_ie(const struct dect_handle *dh, uint8_t type,
		   struct dect_ie_common **dst,
		   const struct dect_sfmt_ie *ie)
{
	const struct dect_ie_handler *ieh;
	int err = -1;

	ieh = &dect_ie_handlers[ie->id];
	if (ieh->parse == NULL)
		goto err1;

	if (ieh->size > 0) {
		*dst = dect_ie_alloc(dh, ieh->size);
		if (*dst == NULL)
			goto err1;
	}

	sfmt_debug("  IE: <<%s>> id: %x len: %u dst: %p\n",
		   ieh->name, ie->id, ie->len, *dst);

	err = ieh->parse(dh, dst, ie);
	if (err < 0)
		goto err2;
	if (ieh->dump != NULL)
		ieh->dump(*dst);
	return 0;

err2:
	dect_free(dh, *dst);
	*dst = NULL;
err1:
	sfmt_debug("smsg: IE parsing error\n");
	return err;
}
EXPORT_SYMBOL(dect_parse_sfmt_ie);

static void sfmt_debug_msg(const struct dect_sfmt_msg_desc *mdesc, const char *msg)
{
	char buf[strlen(mdesc->name) + 1];
	unsigned int i;

	strcpy(buf, mdesc->name);
	for (i = 0; i < sizeof(buf); i++) {
		if (islower(buf[i]))
			buf[i] = toupper(buf[i]);
		if (buf[i] == '_')
			buf[i] = '-';
	}
	sfmt_debug("%s {%s} message\n", msg, buf);
}

enum dect_sfmt_error dect_parse_sfmt_msg(const struct dect_handle *dh,
					 const struct dect_sfmt_msg_desc *mdesc,
					 struct dect_msg_common *_dst,
					 struct dect_msg_buf *mb)
{
	const struct dect_sfmt_ie_desc *desc = mdesc->ie;
	struct dect_ie_common **dst = &_dst->ie[0];
	struct dect_sfmt_ie _ie[2], *ie;
	uint8_t idx = 0;

	sfmt_debug_msg(mdesc, "parse");

	dect_msg_ie_init(desc, dst);
	while (mb->len > 0) {
		/* Parse the next information element header */
		ie = &_ie[idx++ % array_size(_ie)];;
		if (dect_parse_sfmt_ie_header(ie, mb) < 0)
			return -1;

		/* Locate a matching member in the description and apply
		 * policy checks. */
		while (1) {
			if (desc->flags & DECT_SFMT_IE_END)
				goto out;

			switch (dect_rx_status(dh, desc)) {
			case DECT_SFMT_IE_MANDATORY:
				if (desc->type == ie->id)
					goto found;
				return DECT_SFMT_MANDATORY_IE_MISSING;
			case DECT_SFMT_IE_NONE:
				if (desc->type == ie->id)
					return -1;
				break;
			case DECT_SFMT_IE_OPTIONAL:
				if (desc->type == ie->id)
					goto found;
				if (desc->type == DECT_IE_SINGLE_DISPLAY &&
				    ie->id == DECT_IE_MULTI_DISPLAY)
					goto found;
				if (desc->type == DECT_IE_SINGLE_KEYPAD &&
				    ie->id == DECT_IE_MULTI_KEYPAD)
					goto found;
				break;
			}

			dst = dect_next_ie(desc, dst);
			desc++;
			dect_msg_ie_init(desc, dst);
		}
found:
		/* Treat empty variable length IEs as absent */
		if (!(ie->id & DECT_SFMT_IE_FIXED_LEN) && ie->len == 2) {
			sfmt_debug("  IE: <<%s>> id: %x len: %u (empty)\n",
				   dect_ie_handlers[ie->id].name, ie->id, ie->len);
			goto next;
		}

		/* Ignore corrupt optional IEs */
		if (dect_parse_sfmt_ie(dh, desc->type, dst, ie) < 0 &&
		    dect_rx_status(dh, desc) == DECT_SFMT_IE_MANDATORY)
			return DECT_SFMT_MANDATORY_IE_ERROR;

next:
		dect_mbuf_pull(mb, ie->len);

		dst = dect_next_ie(desc, dst);
		desc++;
		dect_msg_ie_init(desc, dst);
	}
out:
	while (!(desc->flags & DECT_SFMT_IE_END)) {
		if (dect_rx_status(dh, desc) == DECT_SFMT_IE_MANDATORY)
			return DECT_SFMT_MANDATORY_IE_MISSING;
		dst = dect_next_ie(desc, dst);
		desc++;
		dect_msg_ie_init(desc, dst);
	}

	return DECT_SFMT_OK;
}

/**
 * Construct a S-Format encoded Information Element
 *
 * @param dh		libdect DECT handle
 * @param type		IE type
 * @param mb		message buffer to append IE to
 * @param ie		information element
 *
 * Construct a S-Format encoded Information Element and append it to the message
 * buffer.
 *
 * @return #DECT_SFMT_OK on success or one of the @ref dect_sfmt_error
 * "S-Format error codes" on error.
 */
enum dect_sfmt_error
dect_build_sfmt_ie(const struct dect_handle *dh, uint8_t type,
		   struct dect_msg_buf *mb,
		   const struct dect_ie_common *ie)
{
	const struct dect_ie_handler *ieh;
	struct dect_sfmt_ie dst;
	enum dect_sfmt_error err = 0;

	if (type == DECT_IE_SINGLE_DISPLAY) {
		struct dect_ie_display *display = dect_ie_container(display, ie);
		if (display->len > 1)
			type = DECT_IE_MULTI_DISPLAY;
	}
	if (type == DECT_IE_SINGLE_KEYPAD) {
		struct dect_ie_keypad *keypad = dect_ie_container(keypad, ie);
		if (keypad->len > 1)
			type = DECT_IE_MULTI_KEYPAD;
	}

	ieh = &dect_ie_handlers[type];
	if (ieh->build == NULL)
		goto err1;

	sfmt_debug("  IE: <<%s>> id: %x %p\n", ieh->name, type, ie);
	if (ieh->dump != NULL)
		ieh->dump(ie);

	dst.data = mb->data + mb->len;
	dst.len = 0;
	err = ieh->build(&dst, ie);
	if (err < 0)
		goto err1;

	dect_build_sfmt_ie_header(&dst, type);
	mb->len += dst.len;
	return 0;

err1:
	return err;
}
EXPORT_SYMBOL(dect_build_sfmt_ie);

static enum dect_sfmt_error
__dect_build_sfmt_ie(const struct dect_handle *dh,
		     const struct dect_sfmt_ie_desc *desc,
		     struct dect_msg_buf *mb,
		     const struct dect_ie_common *ie)
{
	if (dect_tx_status(dh, desc) == DECT_SFMT_IE_NONE) {
		sfmt_debug("  IE <%s> id: %x not allowed\n",
			   dect_ie_handlers[desc->type].name, desc->type);
		return DECT_SFMT_INVALID_IE;
	}
	return dect_build_sfmt_ie(dh, desc->type, mb, ie);
}

enum dect_sfmt_error dect_build_sfmt_msg(const struct dect_handle *dh,
					 const struct dect_sfmt_msg_desc *mdesc,
					 const struct dect_msg_common *_src,
					 struct dect_msg_buf *mb)
{
	const struct dect_sfmt_ie_desc *desc = mdesc->ie;
	struct dect_ie_common * const *src = &_src->ie[0], **next, *rsrc;
	struct dect_ie_list *iel;
	enum dect_sfmt_error err;

	sfmt_debug_msg(mdesc, "build");

	while (!(desc->flags & DECT_SFMT_IE_END)) {
		next = dect_next_ie(desc, (struct dect_ie_common **)src);

		if (desc->type == DECT_IE_REPEAT_INDICATOR) {
			iel = (struct dect_ie_list *)src;
			if (iel->list == NULL) {
				desc++;
				goto next;
			}

			/* Add repeat indicator if more than one element on the list */
			if (iel->list->next != NULL) {
				err = __dect_build_sfmt_ie(dh, desc, mb, &iel->common);
				if (err != DECT_SFMT_OK)
					return err;
			}
			desc++;

			dect_assert(desc->flags & DECT_SFMT_IE_REPEAT);
			dect_foreach_ie(rsrc, iel) {
				err = __dect_build_sfmt_ie(dh, desc, mb, rsrc);
				if (err != DECT_SFMT_OK)
					return err;
			}
		} else if (*src != NULL) {
			err = __dect_build_sfmt_ie(dh, desc, mb, *src);
			if (err != DECT_SFMT_OK)
				return err;
		} else {
			if (dect_tx_status(dh, desc) == DECT_SFMT_IE_MANDATORY)
				goto err_mandatory;
		}
next:
		src = next;
		desc++;
	}

	return DECT_SFMT_OK;

err_mandatory:
	sfmt_debug("  IE <%s> id: %x missing\n",
		   dect_ie_handlers[desc->type].name, desc->type);
	return DECT_SFMT_MANDATORY_IE_MISSING;
}

void dect_msg_free(const struct dect_handle *dh,
		   const struct dect_sfmt_msg_desc *mdesc,
		   struct dect_msg_common *msg)
{
	const struct dect_sfmt_ie_desc *desc = mdesc->ie;
	struct dect_ie_common **ie = &msg->ie[0], **next;

	while (!(desc->flags & DECT_SFMT_IE_END)) {
		next = dect_next_ie(desc, ie);
		if (desc->type == DECT_IE_REPEAT_INDICATOR)
			desc++;
		else if (desc->flags & DECT_SFMT_IE_REPEAT)
			dect_ie_list_put(dh, (void *)*ie);
		else if (*ie != NULL)
			__dect_ie_put(dh, *ie);

		ie = next;
		desc++;
	}
}

/** @} */
