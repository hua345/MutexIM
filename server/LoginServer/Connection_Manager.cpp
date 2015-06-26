#include "stdafx.h"
#include "Connection_Manager.h"

void Connection_Manager::start(TSessionPtr err)
{
	sessionSet_.insert(err);
	err->start();
}

void Connection_Manager::stop(TSessionPtr err)
{
	sessionSet_.erase(err);
	err->stop();
}

void Connection_Manager::stop_all()
{
	std::for_each(sessionSet_.begin(), sessionSet_.end(),
		boost::bind(&ssl_Session::stop, _1));
	sessionSet_.clear();
}
