// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEBCONTROLLER_H_
#define WEBCONTROLLER_H_

#include <string>
#include <vector>
#include <Wt/WDllDefs.h>

#ifndef WT_TARGET_JAVA

#ifdef WT_THREADED
#include <boost/thread.hpp>
#include "threadpool/threadpool.hpp"
#endif

#endif // WT_TARGET_JAVA

namespace Wt {

class CgiParser;
class Configuration;
class EntryPoint;

class WebRequest;
class WebSession;
class WebStream;

class WApplication;
class WWidget;
class WObject;
class WResource;
class WSocketNotifier;
class WStatelessSlot;
class WWebWidget;

/*
 * The controller is a singleton class
 *
 * It either listens for incoming requests from a webstream, using run(),
 * or it may be used to handle an incoming request, using handleRequest().
 * In the latter case, sessions will only expire with a delay -- at the
 * next request. Seems harmless to me.
 *
 * There is a method forceShutDown() to quit the controller.
 *
 * It has the following tasks:
 *  - handle session life-cycle: create new sessions, delete quit()ed
 *    sessions, expire sessions on timeout
 *  - forward the request to the proper session
 *  - manage concurrency
 */
class WT_API WebController
{
public:
  WApplication *doCreateApplication(WebSession *session);
  Configuration& configuration();

#ifndef WT_TARGET_JAVA
  /*
   * Construct the WebController and let it read requests from the given
   * streams.
   */
  WebController(Configuration& configuration, WebStream *stream,
		std::string singleSessionId = std::string());

  ~WebController();

  void run();
  int sessionCount() const;

  void handleRequest(WebRequest *request, const EntryPoint *entryPoint = 0);

  void forceShutdown();

  static std::string appSessionCookie(std::string url);
  static std::string sessionFromCookie(std::string cookies,
				       std::string scriptName,
				       int sessionIdLength);

  static WebController *instance() { return instance_; }

  typedef std::map<int, WSocketNotifier *> SocketNotifierMap;

  void addSocketNotifier(WSocketNotifier *notifier);
  void removeSocketNotifier(WSocketNotifier *notifier);

  // returns false if removeSocketNotifier was called while processing
  bool socketSelected(int descriptor);

  std::string switchSession(WebSession *session,
			    const std::string& newSessionId);

  const SocketNotifierMap& socketNotifiers() const { return socketNotifiers_; }

  std::string generateNewSessionId(WebSession *session);

private:
  Configuration& conf_;
  WebStream     *stream_;
  std::string    singleSessionId_;
  bool           running_;

  typedef std::map<std::string, WebSession *> SessionMap;
  SessionMap sessions_;

  SocketNotifierMap socketNotifiers_;

  bool shutdown_;

#ifdef WT_THREADED
  // mutex to protect access to the sessions map.
  boost::recursive_mutex mutex_;

  boost::threadpool::pool threadPool_;
#endif

  bool expireSessions(std::vector<WebSession *>& toKill);
  void removeSession(const std::string& sessionId);
  void handleRequestThreaded(WebRequest *request);

  const EntryPoint *getEntryPoint(const std::string& deploymentPath);

  static void mxml_error_cb(const char *message);

  static WebController *instance_;

#endif // WT_TARGET_JAVA
};

extern void WebStreamAddSocketNotifier(WSocketNotifier *notifier);
extern void WebStreamRemoveSocketNotifier(WSocketNotifier *notifier);

}

#endif // WEBCONTROLLER_H_
