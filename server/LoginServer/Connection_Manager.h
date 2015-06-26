#pragma once
#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H
#include <set>
#include <boost/noncopyable.hpp>
#include "ssl_Session.h"
/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class Connection_Manager : private boost::noncopyable
{
public:
	/// Add the specified connection to the manager and start it.
	void start(TSessionPtr c);

	/// Stop the specified connection.
	void stop(TSessionPtr c);

	/// Stop all connections.
	void stop_all();
private:
	/// The managed connections.
	std::set<TSessionPtr> sessionSet_;
};

#endif	//CONNECTION_MANAGER_H
