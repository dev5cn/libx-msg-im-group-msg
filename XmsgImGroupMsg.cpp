/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <libx-msg-im-group-pb.h>
#include "XmsgImGroupMsg.h"
#include "mgr/XmsgImMgrNeNetLoad.h"
#include "mgr/XmsgImMgrNeXscServerQuery.h"
#include "mgr/XmsgImMgrNeXscWorkerCount.h"
#include "msg/XmsgImGroupCreate.h"
#include "msg/XmsgImGroupInfoQuery.h"
#include "msg/XmsgImGroupInfoUpdate.h"
#include "msg/XmsgImGroupMemberAdd.h"
#include "msg/XmsgImGroupMemberDelete.h"
#include "msg/XmsgImGroupMemberQuery.h"
#include "msg/XmsgImGroupMemberUpdate.h"
#include "msg/XmsgImGroupMsgQuery.h"
#include "msg/XmsgImGroupMsgRead.h"
#include "msg/XmsgImGroupMsgRecvCfg.h"
#include "msg/XmsgImGroupMsgSync.h"
#include "msg/XmsgImGroupSync.h"
#include "msg/XmsgImGroupSyncGroup.h"
#include "msg/XmsgImGroupUsrGroupQuery.h"
#include "msg/send/XmsgImGroupSendMsg.h"
#include "ne/XmsgApClientEstb.h"
#include "ne/XmsgApClientLost.h"
#include "ne/XmsgImGroupChannelStatusSub.h"
#include "ne/XmsgImGroupObjInfoQuery.h"
#include "ne/XmsgImHlrUsrInit.h"
#include "ne/XmsgNeAuth.h"

XmsgImGroupMsg::XmsgImGroupMsg()
{

}

void XmsgImGroupMsg::init(shared_ptr<XmsgImN2HMsgMgr> priMsgMgr)
{
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupInfoUpdateReq, XmsgImGroupInfoUpdateRsp, XmsgImGroupInfoUpdate::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupMemberAddReq, XmsgImGroupMemberAddRsp, XmsgImGroupMemberAdd::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupMemberDeleteReq, XmsgImGroupMemberDeleteRsp, XmsgImGroupMemberDelete::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupMemberUpdateReq, XmsgImGroupMemberUpdateRsp, XmsgImGroupMemberUpdate::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupMsgQueryReq, XmsgImGroupMsgQueryRsp, XmsgImGroupMsgQuery::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupSendMsgReq, XmsgImGroupSendMsgRsp, XmsgImGroupSendMsg::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupCreateReq, XmsgImGroupCreateRsp, XmsgImGroupCreate::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupInfoQueryReq, XmsgImGroupInfoQueryRsp, XmsgImGroupInfoQuery::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupMemberQueryReq, XmsgImGroupMemberQueryRsp, XmsgImGroupMemberQuery::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupMsgRecvCfgReq, XmsgImGroupMsgRecvCfgRsp, XmsgImGroupMsgRecvCfg::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupMsgSyncReq, XmsgImGroupMsgSyncRsp, XmsgImGroupMsgSync::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupMsgReadReq, XmsgImGroupMsgReadRsp, XmsgImGroupMsgRead::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupSyncReq, XmsgImGroupSyncRsp, XmsgImGroupSync::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupSyncGroupReq, XmsgImGroupSyncGroupRsp, XmsgImGroupSyncGroup::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImGroupUsrGroupQueryReq, XmsgImGroupUsrGroupQueryRsp, XmsgImGroupUsrGroupQuery::handle)
	X_MSG_N2H_PRPC_BEFOR_AUTH(priMsgMgr, XmsgNeAuthReq, XmsgNeAuthRsp, XmsgNeAuth::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeNetLoadReq, XmsgImMgrNeNetLoadRsp, XmsgImMgrNeNetLoad::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeXscServerQueryReq, XmsgImMgrNeXscServerQueryRsp, XmsgImMgrNeXscServerQuery::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeXscWorkerCountReq, XmsgImMgrNeXscWorkerCountRsp, XmsgImMgrNeXscWorkerCount::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImGroupChannelStatusSubReq, XmsgImGroupChannelStatusSubRsp, XmsgImGroupChannelStatusSub::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImGroupObjInfoQueryReq, XmsgImGroupObjInfoQueryRsp, XmsgImGroupObjInfoQuery::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgImHlr, XmsgImHlrUsrInitReq, XmsgImHlrUsrInitRsp, XmsgImHlrUsrInit::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH_UNI(XmsgImHlr, XmsgApClientEstbNotice, XmsgApClientEstb::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH_UNI(XmsgImHlr, XmsgApClientLostNotice, XmsgApClientLost::handle)
}

XmsgImGroupMsg::~XmsgImGroupMsg()
{

}

