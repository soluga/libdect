/*
 * DECT Mobility Management (MM)
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/dect.h>

#include <libdect.h>
#include <utils.h>
#include <s_fmt.h>
#include <lce.h>
#include <mm.h>

static DECT_SFMT_MSG_DESC(mm_access_rights_accept,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_MANDATORY, IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ZAP_FIELD,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SERVICE_CLASS,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_request,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_reject,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_reject,
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	//DECT_SFMT_IE(S_VL_IE_AUTH_REJECT_PARAMETER,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_reply,
	DECT_SFMT_IE(S_VL_IE_RES,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ZAP_FIELD,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SERVICE_CLASS,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_KEY,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_request,
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RAND,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RES,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_key_allocate,
	DECT_SFMT_IE(S_VL_IE_ALLOCATION_TYPE,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_RAND,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_accept,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_SE_IE_USE_TPUI,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_reject,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_request,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign_ack,
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign_rej,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

#define mm_debug(fmt, args...) \
	dect_debug("MM: " fmt "\n", ## args)

static void dect_mm_transaction_destroy(struct dect_handle *dh,
					struct dect_mm_transaction *mmta)
{
	dect_free(dh, mmta);
}

static struct dect_mm_transaction *dect_mm_transaction_alloc(const struct dect_handle *dh)
{
	struct dect_mm_transaction *mmta;

	mmta = dect_zalloc(dh, sizeof(*dh));
	if (mmta == NULL)
		goto err1;
	return mmta;

err1:
	return NULL;
}

static int dect_mm_send_msg(struct dect_handle *dh,
			    const struct dect_mm_transaction *mmta,
			    const struct dect_sfmt_msg_desc *desc,
			    const struct dect_msg_common *msg,
			    enum dect_mm_msg_types type)
{
	return dect_lce_send(dh, &mmta->transaction, desc, msg, type);
}

int dect_mm_access_rights_req(struct dect_handle *dh,
			      struct dect_mm_transaction *mmta,
			      const struct dect_mm_access_rights_param *param)
{
	static struct dect_transaction transaction;
	struct dect_ipui ipui;
	struct dect_mm_access_rights_request_msg msg = {
		.portable_identity	= param->portable_identity,
		.auth_type		= param->auth_type,
		.cipher_info		= param->cipher_info,
		.setup_capability	= NULL,
		.terminal_capability	= param->terminal_capability,
		.model_identifier	= param->model_identifier,
		.codec_list		= NULL,
		.escape_to_proprietary	= NULL,
	};

	mm_debug("ACCESS_RIGHTS-req");
	transaction.pd = DECT_S_PD_MM;

	if (dect_open_transaction(dh, &transaction, &ipui) < 0)
		goto err1;

	if (dect_lce_send(dh, &transaction, &mm_access_rights_request_msg_desc,
			  &msg.common, DECT_MM_ACCESS_RIGHTS_REQUEST) < 0)
		goto err2;
	return 0;

err2:
	dect_close_transaction(dh, &transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return -1;
}

int dect_mm_access_rights_res(struct dect_handle *dh,
			      struct dect_mm_transaction *mmta, bool accept,
			      const struct dect_mm_access_rights_param *param)
{
	struct dect_mm_access_rights_accept_msg msg = {
		.portable_identity	= param->portable_identity,
		.fixed_identity		= param->fixed_identity,
		.auth_type		= param->auth_type,
		.location_area		= param->location_area,
		.cipher_info		= param->cipher_info,
		.setup_capability	= NULL,
		.model_identifier	= param->model_identifier,
		//.iwu_to_iwu		= param->iwu_to_iwu,
		//.codec_list		= param->codec_list,
		//.escape_to_proprietary= param->escape_to_proprietary,
	};
	struct dect_ie_fixed_identity fixed_identity;
	int err;

	if (param->fixed_identity.list == NULL) {
		fixed_identity.type = DECT_FIXED_ID_TYPE_PARK;
		fixed_identity.ari = dh->pari;
		fixed_identity.rpn = 0;
		dect_ie_list_add(&fixed_identity, &msg.fixed_identity);
	}

	mm_debug("ACCESS_RIGHTS-res");
	err = dect_mm_send_msg(dh, mmta, &mm_access_rights_accept_msg_desc,
			       &msg.common, DECT_MM_ACCESS_RIGHTS_ACCEPT);

	dect_close_transaction(dh, &mmta->transaction, DECT_DDL_RELEASE_PARTIAL);
	dect_mm_transaction_destroy(dh, mmta);
	return err;
}

static void dect_mm_rcv_access_rights_request(struct dect_handle *dh,
					      const struct dect_transaction *req,
					      struct dect_msg_buf *mb)
{
	struct dect_mm_access_rights_request_msg msg;
	struct dect_mm_access_rights_param param;
	struct dect_mm_transaction *mmta;

	mm_debug("ACCESS-RIGHTS-REQUEST");
	if (dect_parse_sfmt_msg(dh, &mm_access_rights_request_msg_desc,
				&msg.common, mb) < 0)
		return;

	mmta = dect_mm_transaction_alloc(dh);
	if (mmta == NULL)
		goto err1;

	dect_confirm_transaction(dh, &mmta->transaction, req);

	memset(&param, 0, sizeof(param));
	param.portable_identity   = msg.portable_identity;
	param.auth_type		  = msg.auth_type;
	param.cipher_info	  = msg.cipher_info;
	param.terminal_capability = msg.terminal_capability;

	dh->ops->mm_ops->mm_access_rights_ind(dh, mmta, &param);
err1:
	dect_msg_free(dh, &mm_access_rights_request_msg_desc, &msg.common);
}

static void dect_mm_rcv_access_rights_reject(struct dect_handle *dh,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_access_rights_reject_msg msg;

	if (dect_parse_sfmt_msg(dh, &mm_access_rights_reject_msg_desc, &msg.common, mb) < 0)
		return;
}

static int dect_mm_send_locate_accept(struct dect_handle *dh,
				      struct dect_mm_transaction *mmta,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_accept_msg msg = {
		.portable_identity	= param->portable_identity,
		.location_area		= param->location_area,
		.nwk_assigned_identity	= param->nwk_assigned_identity,
		.duration		= param->duration,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.model_identifier	= param->model_identifier,
	};

	return dect_mm_send_msg(dh, mmta, &mm_locate_accept_msg_desc,
				&msg.common, DECT_MM_LOCATE_ACCEPT);
}

static int dect_mm_send_locate_reject(struct dect_handle *dh,
				      struct dect_mm_transaction *mmta,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_reject_msg msg = {
		.reject_reason		= param->reject_reason,
		.duration		= param->duration,
		.segmented_info		= {},
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= NULL,
	};

	return dect_mm_send_msg(dh, mmta, &mm_locate_reject_msg_desc,
				&msg.common, DECT_MM_LOCATE_REJECT);
}

int dect_mm_locate_res(struct dect_handle *dh, struct dect_mm_transaction *mmta,
		       const struct dect_mm_locate_param *param)
{
	if (param->reject_reason == NULL)
		return dect_mm_send_locate_accept(dh, mmta, param);
	else
		return dect_mm_send_locate_reject(dh, mmta, param);
}

static void dect_mm_locate_ind(struct dect_handle *dh,
			       struct dect_mm_transaction *mmta,
			       const struct dect_mm_locate_request_msg *msg)
{
	struct dect_mm_locate_param *param;

	param = (void *)dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->portable_identity	= dect_ie_hold(msg->portable_identity),
	param->fixed_identity		= dect_ie_hold(msg->fixed_identity),
	param->location_area		= dect_ie_hold(msg->location_area),
	param->nwk_assigned_identity	= dect_ie_hold(msg->nwk_assigned_identity),
	param->cipher_info		= dect_ie_hold(msg->cipher_info),
	param->setup_capability		= dect_ie_hold(msg->setup_capability),
	param->terminal_capability	= dect_ie_hold(msg->terminal_capability),
	param->iwu_to_iwu		= dect_ie_hold(msg->iwu_to_iwu),
	param->model_identifier		= dect_ie_hold(msg->model_identifier),

	dh->ops->mm_ops->mm_locate_ind(dh, mmta, param);
	dect_ie_collection_put(dh, param);
}

static void dect_mm_rcv_locate_request(struct dect_handle *dh,
				       const struct dect_transaction *req,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_locate_request_msg msg;
	struct dect_mm_transaction *mmta;

	mm_debug("LOCATE-REQUEST");
	if (dect_parse_sfmt_msg(dh, &mm_locate_request_msg_desc, &msg.common, mb) < 0)
		goto err1;

	mmta = dect_mm_transaction_alloc(dh);
	if (mmta == NULL)
		goto err2;
	dect_confirm_transaction(dh, &mmta->transaction, req);

	dect_mm_locate_ind(dh, mmta, &msg);
err2:
	dect_msg_free(dh, &mm_locate_request_msg_desc, &msg.common);
err1:
	return;
}

static void dect_mm_rcv_locate_accept(struct dect_handle *dh,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_locate_accept_msg msg;

	mm_debug("LOCATE-ACCEPT");
	if (dect_parse_sfmt_msg(dh, &mm_locate_accept_msg_desc, &msg.common, mb) < 0)
		return;
}

static void dect_mm_rcv_locate_reject(struct dect_handle *dh,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_locate_reject_msg msg;

	mm_debug("LOCATE-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_locate_reject_msg_desc, &msg.common, mb) < 0)
		return;
}

static void dect_mm_rcv_temporary_identity_assign_ack(struct dect_handle *dh,
						      struct dect_mm_transaction *mmta,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_temporary_identity_assign_ack_msg msg;
	struct dect_mm_identity_assign_param param;

	mm_debug("TEMPORARY-IDENTITY-ASSIGN-ACK");
	if (dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_ack_msg_desc,
				&msg.common, mb) < 0)
		return;

	memset(&param, 0, sizeof(param));
	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mmta, true, &param);
}

static void dect_mm_rcv_temporary_identity_assign_rej(struct dect_handle *dh,
						      struct dect_mm_transaction *mmta,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_temporary_identity_assign_rej_msg msg;
	struct dect_mm_identity_assign_param param;

	mm_debug("TEMPORARY-IDENTITY-ASSIGN-REJ");
	if (dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_rej_msg_desc,
				&msg.common, mb) < 0)
		return;

	memset(&param, 0, sizeof(param));
	param.reject_reason = msg.reject_reason;
	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mmta, false, &param);
}

static void dect_mm_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			struct dect_msg_buf *mb)
{
	struct dect_mm_transaction *mmta;

	mmta = container_of(ta, struct dect_mm_transaction, transaction);
	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
	case DECT_MM_AUTHENTICATION_REPLY:
	case DECT_MM_KEY_ALLOCATE:
	case DECT_MM_AUTHENTICATION_REJECT:
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
		break;
	case DECT_MM_ACCESS_RIGHTS_ACCEPT:
		break;
	case DECT_MM_ACCESS_RIGHTS_REJECT:
		return dect_mm_rcv_access_rights_reject(dh, mb);
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REQUEST:
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_ACCEPT:
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REJECT:
	case DECT_MM_CIPHER_REQUEST:
	case DECT_MM_CIPHER_SUGGEST:
	case DECT_MM_CIPHER_REJECT:
	case DECT_MM_INFO_REQUEST:
	case DECT_MM_INFO_ACCEPT:
	case DECT_MM_INFO_SUGGEST:
	case DECT_MM_INFO_REJECT:
		break;
	case DECT_MM_LOCATE_ACCEPT:
		return dect_mm_rcv_locate_accept(dh, mb);
	case DECT_MM_LOCATE_REJECT:
		return dect_mm_rcv_locate_reject(dh, mb);
	case DECT_MM_DETACH:
	case DECT_MM_IDENTITY_REQUEST:
	case DECT_MM_IDENTITY_REPLY:
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN:
		break;
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_ACK:
		return dect_mm_rcv_temporary_identity_assign_ack(dh, mmta, mb);
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_REJ:
		return dect_mm_rcv_temporary_identity_assign_rej(dh, mmta, mb);
	}

	mm_debug("receive unknown msg type %x", mb->type);
}

static void dect_mm_open(struct dect_handle *dh,
			 const struct dect_transaction *req,
			 struct dect_msg_buf *mb)
{
	dect_debug("MM: unknown transaction msg type: %x\n", mb->type);

	switch (mb->type) {
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
		return dect_mm_rcv_access_rights_request(dh, req, mb);
	case DECT_MM_LOCATE_REQUEST:
		return dect_mm_rcv_locate_request(dh, req, mb);
	default:
		break;
	}
}

static void dect_mm_shutdown(struct dect_handle *dh,
			     struct dect_transaction *ta)
{
	struct dect_mm_transaction *mmta;

	mmta = container_of(ta, struct dect_mm_transaction, transaction);
	mm_debug("shutdown");
	dect_close_transaction(dh, &mmta->transaction, DECT_DDL_RELEASE_NORMAL);
}

static const struct dect_nwk_protocol mm_protocol = {
	.name			= "Mobility Management",
	.pd			= DECT_S_PD_MM,
	.max_transactions	= 1,
	.open			= dect_mm_open,
	.shutdown		= dect_mm_shutdown,
	.rcv			= dect_mm_rcv,
};

static void __init dect_mm_init(void)
{
	dect_lce_register_protocol(&mm_protocol);
}
