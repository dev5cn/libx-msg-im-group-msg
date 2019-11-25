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

#include <libx-msg-im-group-db.h>
#include "XmsgImHlrUsrInit.h"

XmsgImHlrUsrInit::XmsgImHlrUsrInit()
{

}

void XmsgImHlrUsrInit::handle(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgImHlrUsrInitReq> req)
{
	if (req->cgt().empty())
	{
		LOG_DEBUG("channel global title can not be null, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "channel global title can not be null");
		return;
	}
	SptrCgt cgt = ChannelGlobalTitle::parse(req->cgt());
	if (cgt == nullptr)
	{
		LOG_DEBUG("channel global title format error, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "channel global title format error: %s", req->cgt().c_str());
		return;
	}
	if (!cgt->isSameHlr(XmsgImGroupCfg::instance()->hlr))
	{
		LOG_DEBUG("this is a foreign channel global title, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORBIDDEN, "this is a foreign channel global title: %s", cgt->toString().c_str());
		return;
	}
	if (XmsgImGroupMgr::instance()->findUsr(cgt) != nullptr)
	{
		LOG_DEBUG("duplicate operation, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORBIDDEN, "duplicate operation, channel global title: %s", cgt->toString().c_str());
		return;
	}
	ullong sts = DateMisc::dida();
	XmsgImGroupMgr::instance()->initXmsgImGroupMemberUlocal(cgt, [sts, cgt, trans](int ret, const string& desc, SptrUl ul)
	{
		if (ul == nullptr)
		{
			LOG_ERROR("init usr group data failed, elap: %dms, cgt: %s, ret: %d, desc: %s", DateMisc::elapDida(sts), cgt->toString().c_str(), ret, desc.c_str())
			return;
		}
		LOG_INFO("init usr group data successful, elap: %dms, local usr: %s", DateMisc::elapDida(sts), ul->toString().c_str())
		trans->success();
	});
}

XmsgImHlrUsrInit::~XmsgImHlrUsrInit()
{

}

