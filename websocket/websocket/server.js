var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var url = require('url');
var querystring = require('querystring');
var ejs = require('ejs');
var uuid = require('uuid');
var redis = require('redis');
var redisOption = {
    auth_pass: "014006",
    detect_buffers: true
}
//detect_buffers:This option lets you switch between Buffers and Strings on a per-command basis,
//whereas return_buffers applies to every command on a client.
var redisSession = redis.createClient(redisOption);//redis.createClient() = redis.createClient(6379, '127.0.0.1', {})
var redisDB = redis.createClient(redisOption);
//--------------------------Cookie and  session
var MAXAGE = 30 * 60 * 1000;//30分钟更新一次Session;
var REDISAGE = 7 * 24 * 60 * 60;//一个星期登录一次;
var BROWERAGE = 7 * 24 * 60 * 60 * 1000;
var sessionKey = 'session_id';
var sessionObj = {
    'id': '',
    'expire': '',
    'originMaxAge': MAXAGE
};
var cookieSerialize = function (name , val, opt) {
    var pairs = [name + '=' + val];
    opt = opt | {};
    if (opt.maxAge) pairs.push('Max-Age=' + opt.maxAge);
    if (opt.domain) pairs.push('Domain=' + opt.domain);
    if (opt.path) pairs.push('Path=' + opt.path);
    if (opt.httpOnly) pairs.push('HttpOnly');
    if (opt.secure) pairs.push('Secure');
    return pairs.join(';');
}
//-------------------------------------------
var app = express();
////////////////////////////////init///////////////////////////////
//-------------------------------------protobuf-----------------------------
var promise = require('promise');
var fs = require('fs');
var iconv = require('iconv-lite');
var Schema = require('protobuf').Schema;
// "schema" contains all message types defined in buftest.proto|desc.
var schema = new Schema(fs.readFileSync('nodeJSON.desc'));
var MsgData = schema['nodeJSON.MsgData'];
var ServerGroupMsg = schema['nodeJSON.serverGroupMsg'];

//----------------------------------------http-------------------------
var http = require('http');
var httpServer = http.Server(app);
var io = require('socket.io')(httpServer);
//选择数据库
redisDB.SELECT("1", function (err) {
    if (!err) console.log("redis1 init ok!");
});
redisSession.SELECT("2", function (err) {
    if (!err) console.log("redis2 init ok!");
});
//服务器监听端口
httpServer.listen(6180, "127.0.0.1", function () {
    console.log('server run at %s %s', httpServer.address().address, httpServer.address().port);
});

//////////////////////////////////////////////////////////////////////////////////
//maxAge milliseconds
// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.engine('.html', ejs.__express);
app.set('view engine', 'html');
// uncomment after placing your favicon in /public
//app.use(favicon(__dirname + '/public/favicon.ico'));
app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
// 定义静态文件目录
app.use(express.static(path.join(__dirname, 'public')));
// 匹配路径和渲染文件
app.get('/', function (req, res){
    //更新redis数据
    res.render('index', { title: 'Websocket Chat' });
});
app.get('/logout', function (req, res) {

    DeleteSession(req, res, function (req, res) {
        res.redirect('/');
    });
});
app.get('/login', function (req, res) {
    var data = { title: 'Chat Login'};
    //获取查询字符串
    req.query = url.parse(req.url, true).query;
    var err = req.query["error"];
    //查询字符串的键可以出现多次
    if (!Array.isArray(err)) {
        if ('invalidERROR' == err)
            data.ErrorStr = "无效登录";
        else if ('existERROR' == err)
            data.ErrorStr = "用户不存在";
        else if ('passERROR' == err)
            data.ErrorStr = "用户名或密码错误";
        else if ('noLogin' == err)
            data.ErrorStr = "用户未登录或登录失效";
        else if ('SessionERROR' == err)
            data.ErrorStr = "用户与Session未对应";
    }
    res.render('login', data);
});
app.post('/login', function (req, res) {
    if (req.body.userId && req.body.password) {
        redisDB.hgetall('user:' + req.body.userId, function (err, reply) {
            if (!err && null != reply) {//"undefined" != typeof (reply) 
                reply.userId = reply.useId;
                if (req.body.userId == reply.userId && req.body.password == reply.password) {
                    //生成Sessionid
                    req.session = sessionObj;
                    req.session.id = uuid.v4();
                    req.session.expire = (new Date()).getTime() + MAXAGE;
                    //存入redis
                    redisSession.set(reply.userId, JSON.stringify(req.session), redis.print);
                    //一个星期后删除Session;
                    redisSession.expire(reply.userId, REDISAGE);
                    //修改浏览器Cookie
                    res.setHeader('Set-Cookie', [cookieSerialize(sessionKey, req.session.id, { 'maxAge': BROWERAGE ,'path':'/'}), cookieSerialize('userId', req.body.userId , { 'path': '/' })]);
                    res.redirect('/chat');
                }
                else {
                    //用户ID与密码错误
                    res.redirect('/login?error=passERROR');
                }
            }
            else {
                //用户不存在
                res.redirect('/login?error=existERROR');
            }
        });
    }
    else
        //无效提交
         res.redirect('/login?error=invalidERROR');
});
app.get('/register', function (req, res) { 
    res.render('register');
});
app.post('/register', function (req, res) {
    if (req.body.userId && req.body.password && req.body.email) { 
        redisDB.exists('user:' + req.body.userId, function (err, reply) {
            if (!err) {
                if ('0' == reply) {
                    //存入数据库保存用户信息
                    redisDB.hmset("user:" + req.body.userId
                        , 
                        { "useId": req.body.userId
                        , "password": req.body.password
                        , "email": req.body.email
                        , "nickName": req.body.userId
                    }
                    , function (err, reply) { 
                        //回调跳转到主页
                        if (!err) {
                            //生成Sessionid
                            req.session = sessionObj;
                            req.session.id = uuid.v4();
                            req.session.expire = (new Date()).getTime() + MAXAGE;
                            //存入redis
                            redisSession.set(req.body.userId, JSON.stringify(req.session), redis.print);
                            //一个星期后删除Session;
                            redisSession.expire(req.body.userId, REDISAGE);
                            //修改浏览器Cookie
                            res.setHeader('Set-Cookie', [cookieSerialize(sessionKey, req.session.id, { 'maxAge': BROWERAGE , 'path':'/'}), cookieSerialize('userId', req.body.userId)]);
                            res.redirect('/chat');
                        }
                        else
                            console.log(err);
                    });
                }
                else if ('1' == reply)
                    res.redirect("/register?ERROR=EXIST");
                else
                    res.redirect("/register?ERROR=REPLYERROR");
            }
        });
    }
    else {
        res.redirect("/register?ERROR=Invalid");
    }
});
app.get('/checkUserId', function (req, res) {
    //获取查询字符串
    req.query = url.parse(req.url, true).query;
    var userId = req.query["userId"];
    //查询字符串的键可以出现多次
    if (!Array.isArray(userId)) {
        if (true) {
            console.log(userId);
            redisDB.exists('user:' + userId, function (err, reply) {
                if (!err) {
                    if ('0' == reply)
                        res.end("OK");
                    else if ('1' == reply)
                        res.end("EXIST");
                    else
                        res.end('REPLY ERROR');
                }
            });
        }
        else
            res.end("LENGTH ERROR");
    }
    else {
        res.end("QUERY ERROR");
    }
});
app.get('/chat', function (req, res) {
    
    CheckSession(req, res , function (req, res) {
        redisDB.hgetall('user:' + req.cookies.userId, function (err, reply) {
            var user = {};
            console.log('user:' + req.cookies.userId, reply);
            if (!err && null != reply) {
                user.nickName = reply.nickName;
                console.log('user:' + req.cookies.userId, reply);
            }

            user.userId = req.cookies.userId;
            user.sessionId = req.session.id;
            res.render('chat', { 'title': 'Chat', 'user': user });
        });

    });

});
app.get('/data/:dataType', function (req, res) {
    CheckSession(req, res, function (req, res) { 
        if ('friendsList' == req.params.dataType) {
            FriendsList(req.cookies.userId, function (data) {
                res.end(JSON.stringify(data));
            });
        }
        else if ('groupsList' == req.params.dataType) {
            GroupsList(req.cookies.userId, function (data) {
                res.end(JSON.stringify(data));
            });
        }
        else if ('groupMember' == req.params.dataType) {
            if (req.query.groupId) {
                GroupMember(req.query.groupId, function (data) {
                    res.end(JSON.stringify(data));
                });
            }
            else { 
                res.end('');
            }
        }
        else if ('unreadPeerMsg' == req.params.dataType) {
            UnreadPeerMsg(req.cookies.userId, function (data) {
                res.setHeader('Content-Type', 'text/text; charset=utf-8');
                // res.end(JSON.stringify(data));
                //var utf8Str = "你好";
                //var gbkStr = iconv.encode(utf8Str, 'gbk');
                res.end(JSON.stringify(data));
            });
        }
        else if ('unreadGroupMsg' == req.params.dataType) {
            UnreadGroupMsg(req.cookies.userId, function (data) {
                res.setHeader('Content-Type', 'text/text; charset=utf-8')
                res.end(JSON.stringify(data));
            });
        }
        else {
            res.send('dataType : ' + req.params.dataType);
        }
    });
});
function UnreadGroupMsg(userId, func) {
    //查询各个群未读消息数目;
    //COMMAND: SORT groupSet:14006 GET # GET unreadNumber:14006:*

    redisDB.sort("groupSet:" + userId, "GET", "#", "GET", "unreadNumber:" + userId + ":*", function (err, reply) {
        if (!err && null != reply) {
            //输出数据;
            var outData = [];
            //查询次数
            var queryNum = 0;
            for (var i = 0; i < reply.length / 2; i++) {
                if (reply[2 * i + 1] >= 1)
                    queryNum += 1;
            }
            //如果没有未读消息跳出循环;
            if (queryNum <= 0)
                func(outData);
            //reply 数据第一个是groupId第二个是未读群消息数目;
            for (var i = 0; i < reply.length / 2; i++) {
                if (reply[2 * i + 1] >= 1) {
                    //查询未读消息
                    //LRANGE groupMsg:1  0  number 
                    redisDB.lrange(new Buffer('groupMsg:' + reply[2 * i]), 0, reply[2 * i + 1] - 1, function (err, msgArr) {
                        if (!err && null != msgArr) {
                            for (var j = 0; j < msgArr.length; j++) {
                                //读出二进制数据后转换为JSON数据
                                var outObj = MsgData.parse(msgArr[j]);
                                //将GBK码转为UTF-8
                                outObj.msgData = iconv.decode(outObj.msgData, 'gbk').toString();
                                //console.log(outObj);
                                //将消息存入数组
                                outData.push(outObj);
                            }
                            queryNum -= 1;
                            if (queryNum <= 0) {
                                func(outData); 
                            }
 
                        }
                    });
                    //将未读消息数目置0;
                    //Set unreadNumber:14006:1 0
                    redisDB.set('unreadNumber:' + userId + reply[2 * i], 0, redis.print);
                }
            }
        }
        else {
            func([]);
        }
    });
}
function UnreadPeerMsg(userId, func) {
    //LRANGE unreadMsg:14006 0 -1;binary data!
    redisDB.lrange(new Buffer('unreadMsg:' + userId), '0', '-1', function (err, reply) {
        if (!err && null != reply) {
            var outData = [];
            for (var i = 0; i < reply.length; i++) {
                //读出二进制数据后转换为JSON数据
                var outObj = MsgData.parse(reply[0]);
                //将GBK码转为UTF-8
                outObj.msgData = iconv.decode(outObj.msgData, 'gbk').toString();
                outData.push(outObj);
            }
            //删除这个Key
            redisDB.DEL('unreadMsg:' + userId, redis.print);
            console.log(outData);       
            func(outObj);
        }
        else {
            func([]);
        }
    });
    
}
//获取群列表;
function GroupMember(groupId, func) {
    //sort groupMember:1 get # get user:*->nickName get user:*->imageurl
    redisDB.sort('groupMember:' + groupId , 'GET', '#', 'GET', 'user:*->nickName', 'GET', 'user:*->imageurl', function (err, reply) {
        if (!err && null != reply) {
            func(reply);
        }
        else {
            func([]);
        }
    });
};
//获取群列表;
function GroupsList(userId, func) {
    //sort groupSet:14006 get # get group:*->nickName get group:*->imageurl
    redisDB.sort('groupSet:' + userId , 'GET', '#', 'GET', 'group:*->groupName', 'GET', 'group:*->imageUrl', function (err, reply) {
        if (!err && null != reply) {
            func(reply);
        }
        else {
            func([]);
        }
    });
};
//获得好友列表
function FriendsList(userId, func)
{
    //sort friendsSet:14006 get # get user:*->nickName get user:*->imageurl
    redisDB.sort('friendsSet:' + userId ,'GET','#','GET','user:*->nickName','GET','user:*->imageurl', function (err, reply) {
        if (!err && null != reply) {
            func(reply);
        }
        else {
            func([]);
        }
    });
};
//删除Session
function DeleteSession(req, res, func) { 
    if (req.cookies.userId) {
        redisSession.del(req.cookies.userId, redis.print);
        res.setHeader('Set-Cookie', cookieSerialize(sessionKey, '', { 'maxAge': 0 , 'path': '/'}));     
    }
    //处理传入函数
    func(req, res);
};
//检测用户ID和Session
function CheckSession(req, res, func){
    if (req.cookies.userId && req.cookies[sessionKey]) {
        //读出Session    
        redisSession.get(req.cookies.userId, function (err, reply) {
            //判断Key是否存在
            if (!err && null != reply) {
                //存入时是JSON读出也是JSON数据;
                var obj = JSON.parse(reply);
                if (obj.id == req.cookies[sessionKey]) {
                    //判断Session是否超时
                    if (obj.expire < (new Date().getTime())) {
                        //更新Session
                        UpdateSession(req, res, function () { 
                            //执行函数
                            func(req, res);
                        });
                    }
                    else {
                        req.session = {};
                        req.session.id = obj.id;
                        //执行函数
                        func(req, res);
                    }
                }
                else {
                    //用户ID与Session未对应
                    console.log("%s %s", obj.id,   req.cookies[sessionKey])
                    res.redirect('/login?error=SessionERROR');
                }
            }
            else {
                //用户未登录或上次登录是一个星期前
                console.log('req.cookies.userId %s reply %s', req.cookies.userId, reply);
                res.redirect('/login?error=noInfoERROR');
            }
        });
    }
    else
        res.redirect('/login?error=requestERROR');
};
//更新Sessionid
function UpdateSession(req, res, func) {
    console.log("Update Session");
    req.session = sessionObj;
    req.session.id = uuid.v4();
    req.session.expire = (new Date()).getTime() + MAXAGE;
    //更新redis数据
    redisSession.ttl(req.cookies.userId, function (err, reply) {
        if (reply >= 10) {
            redisSession.set(req.cookies.userId, JSON.stringify(req.session), redis.print);
            redisSession.expire(req.cookies.userId, reply);
            //获取之前设置的Cookie
            var cookies = res.getHeader("Set-Cookie");
            //生成当前要设置的Cookie
            var session = cookieSerialize(sessionKey, req.session.id, { 'maxAge': BROWERAGE, 'path':'/' });
            //判断之前Cookie是否有多个
            cookies = Array.isArray(cookies) ? cookies.concat(session) :[cookies, session];
            res.setHeader("Set-Cookie", cookies);
            //执行函数
            func(req, res);
        }
        else {
            DeleteSession(req, res, function () {
                res.redirect('/login?error=noLogin');
            });
        }
    });
}
