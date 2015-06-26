var redis = require('redis');
var redisOption = {
    auth_pass: "014006",
    detect_buffers: true
}
var redisDB = redis.createClient(redisOption);
var redisSub = redis.createClient(redisOption);
//选择数据库
redisDB.SELECT("1", function (err) {
    if (!err) console.log("redis init ok!");
});
redisSub.SELECT("1", function (err) {
    if (!err) console.log("redisSub init ok!");
});
//服务器套接字
var g_socketStr = null;
function Subscribe(socketStr) {
    g_socketStr = socketStr;
    redisSub.subscribe('login', redis.print);
    redisSub.subscribe('unlogin', redis.print);
    redisSub.subscribe('online', redis.print);
    redisSub.subscribe('offline', redis.print);
    redisSub.subscribe('peerMsg:' + socketStr, redis.print);
    redisSub.subscribe('groupMsg:' + socketStr, redis.print);
}
redisSub.on('message', function (channel, message) {
    if ('online' == channel) {
        console.log("%s 上线了", message);
    }
    else if ('offline' == channel) {
        console.log("%s 离线了", message);
    }
    else if (channel == 'groupMsg:' + g_socketStr) {
        //读出二进制数据后转换为JSON数据
        //{ userId:[], msgData:''}
        var outObj = ServerGroupMsg.parse(message);
        var outMsg = MsgData.parse(outObj.msgData);
        //将GBK码转为UTF-8
        outObj.msgData = iconv.decode(outObj.msgData, 'gbk').toString();
        outObj.type = 'group';
        for (var i = 0; i < outObj.userId.length; i++) {
            var toSocket = userIdMap.get(outObj.userId[i]);
            if (undefined != toSocket) {
                console.log("%s %s", channel, JSON.stringify(outObj));
                toSocket.emit('group message', outObj);
            }
        }
    }
    else if (channel == 'peerMsg:' + g_socketStr) {
        //读出二进制数据后转换为JSON数据
        var outObj = MsgData.parse(message);
        //将GBK码转为UTF-8
        outObj.msgData = iconv.decode(outObj.msgData, 'gbk').toString();
        outObj.type = 'peer';
        var toSocket = userIdMap.get(outObj.toId);
        if (undefined != toSocket) {
            console.log("%s %s", channel, JSON.stringify(outObj));
            toSocket.emit('peer message', outObj);
        }
    }
});
//-------------------------------------protobuf-----------------------------
var promise = require('promise');
var fs = require('fs');
var iconv = require('iconv-lite');
var Schema = require('protobuf').Schema;
// "schema" contains all message types defined in buftest.proto|desc.
var schema = new Schema(fs.readFileSync('nodeJSON.desc'));
var MsgData = schema['nodeJSON.MsgData'];
var ServerGroupMsg = schema['nodeJSON.serverGroupMsg'];
/////////////////////////////////////////////////////////////////////
var http = require('http');
var fs = require('fs');
var httpServer = http.createServer(function (req, res) {
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('Hello World\n');
});
function handler(req, res) {
    fs.readFile(__dirname + '/index.html',
  function (err, data) {
        if (err) {
            res.writeHead(500);
            return res.end('Error loading index.html');
        }
        
        res.writeHead(200);
        res.end(data);
    });
}
var io = require('socket.io')(httpServer);
//服务器监听端口
httpServer.listen(6181, "127.0.0.1", function () {
    console.log('server run at %s %s', httpServer.address().address, httpServer.address().port);
    Subscribe(httpServer.address().address　+ ':' + httpServer.address().port);
});

///////////////////////////////////socket.io////////////////////////////////////////////////
//用户ID对应socket  [userId, socket]
var userIdMap = new Map();
//socket对应用户ID  [socket, userId]
var socketMap = new Map();
//私聊
var chat = io.on('connection', function (socket) {
    socket.emit('a message', {
        'message': 'connection'
    });

    socket.on('peer message', function (data) {
        if (undefined != data.toId) {
            //找到对应的Socket
            var toSocket = userIdMap[data.toId];
            console.log(data);
            //判断对方是否在本服务器
            if (undefined != toSocket) {
                toSocket.emit('peer message', data);
            }
            else {
                data.type = 'CHAT_TEXT';
                redisDB.get('online:' + data.toId, function (err, reply) {
                    if (!err && null != reply) {
                        console.log('用户在服务器' , reply);
                        //发布一条消息
                        data.msgData = iconv.encode(data.msgData, 'gbk');
                        redisDB.publish('peerMsg:' + reply, MsgData.serialize(data));

                    }
                    else {
                        //如果用户未登陆;将消息插入到未读消息列表;	
                        var buf = MsgData.serialize(data);
                        redisDB.lpush('unreadMsg:' + data.toId, buf , function (err, reply) {
                            if (!err && null != reply) {
                                //只保存99条消息;如果超过截取后面99条;
                                if (parseInt(reply) > 99) {
                                    redisDB.ltrim('unreadMsg:' + data.toId, 0, 99, redis.print);
                                }
                            }
                        });
                    }
                });
            }
        }
    });

    socket.on('group message', function (data) {
        if (undefined != data.toId) {
            //给所有服务器Publish消息,并存入数据库;
            data.type = 'GROUPCHAT_TEXT';
            console.log(data);
            //处理群在线的用户消息
            redisDB.sort('onlineGroup:' + data.toId, 'get', '#', 'get', 'online:*', function (err, reply) {
                
                if (!err && null != reply) {
                    //[userid, serverSocket]
                    //javascript的Map简化了很多操作
                    //[serverSocket => [userId, userId]];
                    var dataMap = new Map();
                    for (var i = 0; i < reply.length / 2; i++) {
                        //判断是否在本服务器上
                        if (g_socketStr != reply[2 * i + 1]) {
                            var value = dataMap.get(reply[2 * i]);
                            if (undefined == value) {
                                dataMap.set(reply[2 * i], [reply[2 * i + 1]]);
                            }
                            else {
                                value.push(reply[2 * i + 1]);
                                dataMap.set(reply[2 * i], value);
                            }
                        }
                        else {
                            //如果在本服务器上直接发送
                            var userSocket = userIdMap.get(reply[2 * i]);
                            if (undefined != userSocket) {
                                data.type = 'group'
                                userSocket.emit('group message', data);
                            }
                        }
                    }
                    //对其他服务器Publish消息
                    if (dataMap.size >= 1) {
                        dataMap.forEach(function (value, key, map) {
                            var temp = data;
                            temp.msgData = iconv.encode(data.msgData, 'gbk');
                            var groupMsg = {};
                            groupMsg.userId = value;
                            groupMsg.msgData = MsgData.serialize(temp);
                            //发布一条消息
                            redisDB.publish('groupMsg:' + key, ServerGroupMsg.serialize(data));
                            console.log('Publish groupMsg:' + key);
                        });
                    }
                }
            });
            //处理群未在线的用户消息
            //从左边入列表存储二进制数据;
            redisDB.lpush('groupMsg:' + data.toId, MsgData.serialize(data), function (err, reply) {
                //只保存99条消息;如果超过截取后面99条;
                if (!err && null != reply) {
                    if (parseInt(reply) > 99) {
                        redisDB.ltrim('groupMsg:' + data.toId, 0, 99, redis.print);
                    }
                }
            });
            //返回一个集合的全部成员，该集合是所有给定集合之间的差集。;第一个集合与其他集合的差集;
            redisDB.sdiff('groupMember:' + data.toId, 'onlineGroup:' + data.toId, function (err, reply) {
                if (!err && null != reply) {
                    //reply [userId]
                    //pipeline
                    //Key unreadNumber : userid : groupid ;+ 1;方便查询数据;
                    // start a separate multi command queue 
                    var multi = redisDB.multi();
                    for (var i = 0; i < reply.length; i++) {
                        var key = 'unreadNumber:';
                        key += reply[i];
                        key += ':';
                        key += data.toId;
                        multi.incr(key);
                    }
                    multi.exec(function (err, replies) {
                        console.log(replies);
                    });
                }
            });
        }
    });
    //握手连接
    socket.on('handleShake', function (data) {
        //将用户ID与Socket映射
        if (undefined != data.userId) {
            userIdMap.set(data.userId, socket);
            socketMap.set(socket, data.userId);
            //添加online:userid键值对;
            redisDB.set('online:' + data.userId, g_socketStr);
            //Publish 在线消息;
            redisDB.publish('online', data.userId);
            //找到用户所有的群;
            redisDB.smembers('groupSet:' + data.userId, function (err, reply) {
                if (!err && null != reply) {
                    //对所有的群的onlineGroup:groupid添加成员;
                    for (var i = 0; i < reply.length; i++) {
                        redisDB.sadd('onlineGroup:' + reply[i], data.userId, redis.print);
                    }
                }
            });
            console.log("%s 已经连接", data.userId);
        }
    });
    socket.on('disconnect', function () {
        var nowUserId = socketMap.get(socket);
        if (undefined != nowUserId) {
            console.log('%s disconnected', nowUserId);
            //Publish 离线消息;
            redisDB.publish('offline', nowUserId);
            //删除online:userid键值对;
            redisDB.del("online:" + nowUserId);
            //找到用户所有的群;
            redisDB.smembers('groupSet:' + nowUserId, function (err, reply) {
                if (!err && null != reply) {
                    //对所有的群的onlineGroup:groupid移除成员;
                    for (var i = 0; i < reply.length; i++) {
                        redisDB.srem('onlineGroup:' + reply[i], nowUserId, redis.print);
                    }
                }
            });
            //删除用户ID与Socket映射
            socketMap.delete(socket);
            userIdMap.delete(nowUserId);
        }
    });
});