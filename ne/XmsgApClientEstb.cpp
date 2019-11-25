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

#include "XmsgApClientEstb.h"

XmsgApClientEstb::XmsgApClientEstb()
{

}

void XmsgApClientEstb::handle(shared_ptr<XmsgNeUsr> nu, SptrXitup trans, shared_ptr<XmsgApClientEstbNotice> notice)
{
	SptrUl ul = static_pointer_cast<XmsgImGroupMemberUlocal>(XmsgImGroupMgr::instance()->findUsr(notice->cgt()));
	if (ul == nullptr)
	{
		LOG_ERROR("can not found group local member, notice: %s", notice->ShortDebugString().c_str())
		return;
	}
	shared_ptr<XmsgNeUsr> xnu = XmsgNeMgr::instance()->findByCgt(notice->apcgt());
	if (xnu == nullptr)
	{
		LOG_ERROR("can not found x-msg-ap network element, may be this network element disconnected with it, ap-cgt: %s", notice->apcgt().c_str())
		return;
	}
	shared_ptr<XmsgAp> ap = static_pointer_cast<XmsgAp>(XmsgImChannel::cast(xnu->channel));
	SptrClientLocal client(new XmsgImClientLocal(notice->plat(), notice->did(), ul, notice->ccid(), ap));
	ul->future([ul, client]
	{
		ul->addClient(client); 
		XmsgImClientLocalMgr::instance()->add(client);
		LOG_DEBUG("have a x-msg-im-client attached on x-msg-ap, client: %s", client->toString().c_str())
	});
}

XmsgApClientEstb::~XmsgApClientEstb()
{

}

