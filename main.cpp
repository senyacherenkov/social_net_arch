#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Logger.h"
#include <iostream>
#include <atomic>
#include <fstream>
#include <string>
#include <vector>
#include <set>

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::Data;
using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

std::atomic<bool> startPage{false};
std::atomic<bool> signUp{false};
std::atomic<bool> signIn{false};
std::atomic<bool> profile{false};

constexpr const char* kDbCreds = "host=localhost;port=3306;db=social_net;user=poco_server;"
                                 "password=otus;compress=true;auto-reconnect=true";
constexpr const char* kSql = "MySQL";

struct Profile {
    std::string login_;
    std::string pwd_;
    std::string fname_;
    std::string sname_;
    int32_t age_{0};
    std::string gender_;
    std::string hobbies_;
    std::string city_;
    std::vector<std::pair<std::string, std::string>> friends_;
};

void ReadFile(const std::string & inPath, std::string & str)
{
    std::ifstream t(inPath.c_str());

    if(!t.is_open())
        return;
    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

class HelloRequestHandler: public HTTPRequestHandler
{
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
    {
        Application& app = Application::instance();
        app.logger().information("Request from %s", request.clientAddress().toString());

        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");

        std::string body;
        ReadFile("../greeting.html", body);
        response.setContentLength(body.size());
        response.send() << body;

        startPage.store(true);
        signIn.store(true);
    }
};

class RegistrationRequestHandler: public HTTPRequestHandler
{
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
    {
        Application& app = Application::instance();
        app.logger().information("Request from %s", request.clientAddress().toString());

        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");

        std::string body;
        ReadFile("../registration.html", body);
        response.setContentLength(body.size());
        response.send() << body;

        startPage.store(false);
        signIn.store(false);
        signUp.store(true);
    }
};

class DisplayFriendsRequestHandler: public HTTPRequestHandler
{
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
    {
        Application& app = Application::instance();
        app.logger().information("Request from %s", request.clientAddress().toString());

        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");

        Poco::Net::HTMLForm form(request, request.stream());
        Session session_{kSql, kDbCreds};
        std::set<UInt16> profile_ids;
        std::set<UInt16> friend_ids;
        UInt64 user_id = std::stoi(form.get("userId"));
        if(form.has("friendId")) {
            UInt64 friendId = std::stoi(form.get("friendId"));
            session_ << "INSERT INTO friends (userid, frid) VALUES (?, ?)", use(user_id), use(friendId), now;
        }

        Profile pr{"", "", form.get("fname"), form.get("sname")};
        session_ << "SELECT crid FROM profile WHERE fname=(?) and sname=(?)",
                into(profile_ids), use(pr.fname_), use(pr.sname_), now;
        session_ << "SELECT frid FROM friends WHERE userid=(?)",
                into(friend_ids), use(user_id), now;

        auto& out = response.send();
            out << "<html>"
                    << "<body>"
                        << "<form action=\"http://127.0.0.1:8080\" method=\"post\">"
                            << "<p><b>Results:</b></p>";
        for(auto& pid: profile_ids) {
                        out << "<p>user:  " << pr.fname_ << " " << pr.sname_ <<"<Br>"
                            << "<input type=\"hidden\" name=\"friendId\" value=" << pid << ">"
                            << "<input type=\"hidden\" name=\"userId\" value=" << user_id << ">"
                            << "<input type=\"hidden\" name=\"fname\" value=" << pr.fname_ << ">"
                            << "<input type=\"hidden\" name=\"sname\" value=" << pr.sname_ << ">";
            if(pid == user_id)
                        out << "it's you";
            else if(friend_ids.find(pid) == friend_ids.end())
                        out << "<input type=\"submit\" value=\"add as friend\">";
            else
                        out << "is your friend";
        }
                    out << "</form>"
                    << "</p></body>"
               << "</html>";
    }
};

class DisplayUserRequestHandler: public HTTPRequestHandler
{
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
    {
        Application& app = Application::instance();
        app.logger().information("Request from %s", request.clientAddress().toString());

        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");

        Poco::Net::HTMLForm form(request, request.stream());

        UInt64 cred_id = 0;
        Profile pr;
        pr.login_ = form.get("login");
        pr.pwd_ = form.get("pwd");
        Session session_{kSql, kDbCreds};

        if (signIn.load()) {
            session_ << "SELECT id FROM creds WHERE login=(?) and pwd=(?)",
                    into(cred_id), use(pr.login_), use(pr.pwd_), now;
            if(cred_id) {
                session_ << "SELECT fname, sname, age, gender, hob, city FROM profile WHERE crid=(?)",
                                    into(pr.fname_), into(pr.sname_), into(pr.age_), into(pr.gender_),
                                    into(pr.hobbies_), into(pr.city_), use(cred_id), now;
                std::vector<int> frids;
                session_ << "SELECT frid FROM friends WHERE userid=(?)", into(frids), use(cred_id), now;
                if(!frids.empty()) {
                    for(auto& fid: frids) {
                        std::string f, s;
                        session_ << "SELECT fname, sname FROM profile WHERE id=(?)", into(f), into(s), use(fid), now;
                        pr.friends_.emplace_back(f, s);
                    }
                }
            }
            signIn.store(false);
        } else if(signUp.load()) {
            pr = Profile {form.get("login"), form.get("pwd"), form.get("fname"), form.get("sname"),
                        std::stoi(form.get("age").data()), form.get("gender"), form.get("hobbies"), form.get("city"),};
            std::string login;
            session_ << "SELECT login FROM creds WHERE login=(?)", into(login), use(pr.login_), now;
            if(!login.empty()) {
                response.send()
                        << "<html>"
                            << "<body>"
                                << "<p><b>such login already in use:</b></p>"
                            << "</body>"
                        << "</html>";
                return;
            }
            session_ << "INSERT INTO creds (login, pwd) VALUES (?, ?)", use(pr.login_), use(pr.pwd_), now;
            session_ << "SELECT id FROM creds WHERE login=(?) and pwd=(?)",
                    into(cred_id), use(pr.login_), use(pr.pwd_), now;
            session_ << "INSERT INTO profile (crid, fname, sname, age, gender, hob, city) VALUES (?, ?, ?, ?, ?, ?, ?)",
                    use(cred_id), use(pr.fname_), use(pr.sname_), use(pr.age_), use(pr.gender_),
                    use(pr.hobbies_), use(pr.city_),now;

            signUp.store(false);
        }
        auto& out = response.send();
                out << "<html>"
                    << "<body>"
                        << "<p><b>Profile:</b></p>"
                        << "<p>First name: " << pr.fname_ <<"<Br>"
                        << "Second name: " << pr.sname_ <<"<Br>"
                        << "Gender: " << pr.gender_ <<"<Br>"
                        << "Age: " << pr.age_ <<"<Br>"
                        << "Hobbies: " << pr.hobbies_ <<"<Br>"
                        << "City: " << pr.city_ <<"<Br>"
                        << "Friends: " << "<Br>";
        for(auto& mate: pr.friends_)
                        out << mate.first << " " << mate.second << "<Br>";
                    out << "Search friends: " << "<Br>"
                        << "<form action=\"http://127.0.0.1:8080\" method=\"post\">"
                            << "name "
                            << "<input name=\"fname\" id=\"fname\">"
                            << " surname "
                            << "<input name=\"sname\" id=\"sname\">"
                            << "<input type=\"hidden\" name=\"userId\" value=" << cred_id << "\">"
                            << "<input type=\"submit\" value=\"search\">"
                        << "</form></p>"
                    << "</body>"
                << "</html>";

        profile.store(true);
    }
};

class HelloRequestHandlerFactory: public HTTPRequestHandlerFactory
{
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
    {
        if(request.getMethod().find(HTTPServerRequest::HTTP_GET) != std::string::npos &&
                startPage.load()) {
            return new RegistrationRequestHandler;
        } else if(request.getMethod().find(HTTPServerRequest::HTTP_POST) != std::string::npos) {
                if(signUp.load() || signIn.load())
                    return new DisplayUserRequestHandler;
                if(profile.load())
                    return new DisplayFriendsRequestHandler;
        } else {
            return new HelloRequestHandler;
        }
    }
};

class WebServerApp: public ServerApplication
{
    void initialize(Application& self)
    {
        loadConfiguration();
        ServerApplication::initialize(self);
        dbInit();
    }

    void handlePort(const std::string& name, const std::string& value)
    {
        port_ = std::stoi(value);
    }

    void defineOptions(OptionSet& options)
    {
         Application::defineOptions(options);

         options.addOption(
         Option("port", "p", "port of server")
         .required(true)
         .repeatable(true)
         .argument("port")
         .callback(OptionCallback<WebServerApp>(this, &WebServerApp::handlePort)));
    }
    void dbInit() {
        Poco::Data::MySQL::Connector::registerConnector();

        Session session_{kSql, kDbCreds};

        //session << "CREATE DATABASE social_net", now;
        session_ << "DROP TABLE IF EXISTS creds", now;
        session_ << "DROP TABLE IF EXISTS profile", now;
        session_ << "DROP TABLE IF EXISTS friends", now;

        session_ << "CREATE TABLE creds "
                     "(id             INT unsigned NOT NULL AUTO_INCREMENT,"
                     "login           VARCHAR(150) NOT NULL,"
                     "pwd             VARCHAR(150) NOT NULL,"
                     "PRIMARY KEY     (id));", now;

        session_ << "CREATE TABLE profile "
                   "(id               INT unsigned NOT NULL AUTO_INCREMENT,"
                     "crid            INT unsigned NOT NULL,"
                     "fname           VARCHAR(150) NOT NULL,"
                     "sname           VARCHAR(150) NOT NULL,"
                     "age             INT unsigned NOT NULL,"
                     "gender          VARCHAR(150) NOT NULL,"
                     "hob             VARCHAR(150) NOT NULL,"
                     "city            VARCHAR(150) NOT NULL,"
                     "PRIMARY KEY     (id));", now;

        session_ << "CREATE TABLE friends "
                     "(id             INT unsigned NOT NULL AUTO_INCREMENT,"
                     "userid          INT unsigned NOT NULL,"
                     "frid            INT unsigned NOT NULL,"
                     "PRIMARY KEY     (id));", now;
    }
    int main(const std::vector<std::string>& args)
    {
        UInt16 port = static_cast<UInt16>(config().getUInt("port", port_));

        HTTPServer srv(new HelloRequestHandlerFactory, port);
        srv.start();
        logger().information("HTTP Server started on port %hu.", port);
        waitForTerminationRequest();
        logger().information("Stopping HTTP Server...");
        srv.stop();

        return Application::EXIT_OK;
    }

private:
    int port_{05};
};

POCO_SERVER_MAIN(WebServerApp)
