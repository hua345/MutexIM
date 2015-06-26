          //添加好友列表信息
function AppendFriendsList(data) {
    //userId, nickName, image
    var infoLen = 3;
    dataObj = JSON.parse(data);
    for (var i = 0; i < dataObj.length / infoLen ; i++) {
        var liElem = document.createElement("li");
        liElem.innerHTML = "<img src='image/aa.jpg'><span class='userInfo' data-userId='" + dataObj[infoLen * i] + "'>" + dataObj[infoLen * i + 1] + "</span>";
        contentFriends.appendChild(liElem);
    }
    var friendsList = contentFriends.querySelectorAll('.userInfo');
    for (var i = 0; i < friendsList.length; i++) {
        friendsList[i].addEventListener('click', HandlePeerChat);
    }
}
//添加群列表信息
function AppendGroupList(data) {
    //groupId, groupName, image
    var infoLen = 3;
    dataObj = JSON.parse(data);
    for (var i = 0; i < dataObj.length / infoLen ; i++) {
        var liElem = document.createElement("li");
        liElem.innerHTML = "<img src='image/group.jpg'><span class='userInfo' data-groupId='" + dataObj[infoLen * i] + "'>" + dataObj[infoLen * i + 1] + "</span>";
        contentGroups.appendChild(liElem);
    } 
    var groupsList = contentGroups.querySelectorAll('.userInfo');
    for (var i = 0; i < groupsList.length; i++) {
        groupsList[i].addEventListener('click', HandleGroupChat);
    }

}
//添加Group聊天窗口
function HandleGroupChat(target) {
    target.toElement =  target.srcElement || target.target || target.toElement ; // 获取触发事件的源对象
    var groupId = target.toElement.dataset["groupid"];
    var groupName = target.toElement.textContent || target.toElement.innerText;//获取文本内容
    var isExist = false;
    var liArr = sectionChatTabs.children;
    for (var i = 0; i < liArr.length; i++) {
        if (liArr[i].firstElementChild.dataset["groupid"] == groupId) {
            EventClick(liArr[i].firstElementChild);
            isExist = true;
            break;
        }
    }
    if (!isExist) {
        createGroupDlg(groupId, groupName);
        console.log("%s %s", groupId, groupName);
    }
}
//添加Peer聊天窗口
function HandlePeerChat(target) {
    target.toElement = target.srcElement || target.target || target.toElement; // 获取触发事件的源对象
    var userId = target.toElement.dataset["userid"];
    var nickName = target.toElement.textContent || target.toElement.innerText;//获取文本内容
    var isExist = false;
    var liArr = sectionChatTabs.children;
    for (var i = 0; i < liArr.length; i++) {
        if (liArr[i].firstElementChild.dataset["userid"] == userId) {
            console.log("已经创建了窗口%s %s", userId, nickName);
            EventClick(liArr[i].firstElementChild);
            isExist = true;
            break;
        }
    }
    if (!isExist) {
        createPeerDlg(userId, nickName);
        console.log("%s %s", userId, nickName);
    }
}
//关闭图标的点击事件
function HandleCloseClick(target) {
    target.toElement = target.srcElement || target.target || target.toElement; // 获取触发事件的源对象
    target.preventDefault();
    target.stopPropagation();
    if ("SPAN" == target.toElement.tagName) {
        var aElem = target.toElement.parentElement.firstElementChild;
        //判断当前活动元素是否被关闭
        var isActive = false;
        if ("tab-current" == target.toElement.parentElement.className) {
            isActive = true;
        }
        //删除该聊天窗口
        if ("peer" == aElem.dataset["type"]) {
            console.log("删除 %s", aElem.dataset["userid"]);
            document.getElementById("section-peer-" + aElem.dataset["userid"]).remove();
        }
        else if ("group" == aElem.dataset["type"]) {
            console.log("删除 %s", aElem.dataset["groupid"]);
            document.getElementById("section-group-" + aElem.dataset["groupid"]).remove();
        }
        //删除该聊天窗口Tab
        target.toElement.parentElement.remove();
        if (isActive) {
            console.log("当前活动元素被关闭");
            if (sectionChatTabs.firstElementChild) {
                EventClick(sectionChatTabs.firstElementChild.firstElementChild);
            }

        }

    }
}
//触发点击事件
function EventClick(elem) {
    // IE
    if (document.all) {
        elem.click();
    }
	// 其它浏览器
    else {
        var e = document.createEvent("MouseEvents");
        e.initEvent("click", true, true);
        elem.dispatchEvent(e);
    }
}
//处理Tab点击事件
function HandleChatClick(target) {
    target.toElement = target.srcElement || target.target || target.toElement; // 获取触发事件的源对象
    target.preventDefault();
    target.stopPropagation();
    //设置之前活动的Tab样式为空
    // debugger;
    var lastChat = sectionChatTabs.querySelector(".tab-current");
    if (null != lastChat) {
        //设置Tab样式
        lastChat.className = '';
        //设置Content样式
        if ("peer" == lastChat.firstElementChild.dataset["type"]) {
            document.getElementById("section-peer-" + lastChat.firstElementChild.dataset["userid"]).className = "";
        }
        else if ("group" == lastChat.firstElementChild.dataset["type"]) {
            document.getElementById("section-group-" + lastChat.firstElementChild.dataset["groupid"]).className = "";
        }
    }
    if ("A" == target.toElement.tagName) {
        AElemClick(target);
    }
    else if ("SPAN" == target.toElement.tagName) {
        SpanElemClick(target);
    }
    else if ("LI" == target.toElement.tagName) {
        LiElemClick(target);
    }
}
//点击li标签
function LiElemClick(target) {
    target.toElement = target.srcElement || target.target || target.toElement; // 获取触发事件的源对象
    target.toElement.className = 'tab-current';
    //设置Content样式
    if ("peer" == target.toElement.firstElementChild.dataset["type"]) {
        console.log("Tab 点击: %s", target.toElement.firstElementChild.dataset["userid"]);
        document.getElementById("section-peer-" + target.toElement.firstElementChild.dataset["userid"]).className = 'content-current';
    }
    else if ("group" == target.toElement.firstElementChild.dataset["type"]) {
        console.log("Tab 点击: %s", target.toElement.firstElementChild.dataset["groupid"]);
        document.getElementById("section-group-" + target.toElement.firstElementChild.dataset["groupid"]).className = 'content-current';
    }
}
//点击span标签
function SpanElemClick(target) {
    target.toElement = target.srcElement || target.target || target.toElement; // 获取触发事件的源对象
    target.toElement.parentElement.parentElement.className = 'tab-current';
    //设置Content样式
    if ("peer" == target.toElement.parentElement.dataset["type"]) {
        console.log("Tab 点击: %s", target.toElement.parentElement.dataset["userid"]);
        document.getElementById("section-peer-" + target.toElement.parentElement.dataset["userid"]).className = 'content-current';
    }
    else if ("group" == target.toElement.parentElement.dataset["type"]) {
        console.log("Tab 点击: %s", target.toElement.parentElement.dataset["groupid"]);
        document.getElementById("section-group-" + target.toElement.parentElement.dataset["groupid"]).className = 'content-current';
    }
}
//点击a元素标签
function AElemClick(target) {
    target.toElement = target.srcElement || target.target || target.toElement; // 获取触发事件的源对象
    target.toElement.parentElement.className = 'tab-current';
    //设置Content样式
    if ("peer" == target.toElement.dataset["type"]) {
        console.log("Tab 点击: %s", target.toElement.dataset["userid"]);
        document.getElementById("section-peer-" + target.toElement.dataset["userid"]).className = 'content-current';
    }
    else if ("group" == target.toElement.dataset["type"]) {
        console.log("Tab 点击: %s", target.toElement.dataset["groupid"]);
        document.getElementById("section-group-" + target.toElement.dataset["groupid"]).className = 'content-current';
    }
}
//点击发送按钮
function SendClick(target) {
    target.toElement = target.srcElement || target.target || target.toElement; // 获取触发事件的源对象
    if ("SPAN" == target.toElement.tagName) {
        var textElem = target.toElement.previousElementSibling;
        var data = {};
        data.type = textElem.dataset["type"];//消息类型
        data.timeStamp = new Date().getTime();//当前时间戳
        data.fromId = document.getElementById("myUserId").innerText;//我的ID
        data.msgData = textElem.value;
        if ("group" == textElem.dataset["type"]) {
            data.toId = textElem.dataset["groupid"];
            //添加消息记录
            appendLocalGroupMsg(data);
            g_socket.emit('group message', data);
        }
        else if ("peer" == textElem.dataset["type"]) {
            data.toId = textElem.dataset["userid"];
            //添加消息记录
            appendLocalPeerMsg(data);
            g_socket.emit('peer message', data);
        }
        
        console.log(data);
    }
}
//创建点对点聊天窗口
function createPeerDlg(userId, nickName) {
    //创建窗口Tab标签,并监听点击事件
    var liElem = document.createElement("li");
    liElem.innerHTML = "<a href='#' data-type='peer' data-userId='" + userId + "' class='navIcon icon-friends'><span>" + nickName + "</span></a><span class='close' ></span>";
    
    liElem.firstElementChild.addEventListener("click", HandleChatClick);
    //<span>添加close事件
    liElem.lastElementChild.addEventListener("click", HandleCloseClick);
    sectionChatTabs.appendChild(liElem);
    //创建对应的内容
    var sectionElem = document.createElement("section");
    sectionElem.innerHTML = "<div class='content'></div>";
    sectionElem.id = "section-peer-" + userId;
    //div
    var HtmlStr = "<div class='messages'></div><div class='write-form'><textarea data-type='peer'  data-userid='";
    HtmlStr += userId;
    HtmlStr += "' placeholder=' Type your message ' rows='2'></textarea><span class='send'>发送</span></div>";
    sectionElem.firstElementChild.innerHTML = HtmlStr;
    console.log(sectionElem.firstElementChild.lastElementChild.lastElementChild);
    sectionElem.firstElementChild.lastElementChild.lastElementChild.addEventListener("click", SendClick);
    contentChat.appendChild(sectionElem);
    //触发点击事件，修改样式
    EventClick(sectionChatTabs.lastElementChild.firstElementChild);
}
//创建群聊窗口
function createGroupDlg(groupId, groupName) {
    //创建窗口Tab标签,并监听点击事件
    var liElem = document.createElement("li");
    liElem.innerHTML = "<a href='#' data-type='group' data-groupId='" + groupId + "' class='navIcon icon-groups'><span>" + groupName + "</span></a><span class='close' ></span>";
    
    liElem.firstElementChild.addEventListener("click", HandleChatClick);
    //<span>添加close事件
    liElem.lastElementChild.addEventListener("click", HandleCloseClick);
    sectionChatTabs.appendChild(liElem);
    //创建对应的内容
    var sectionElem = document.createElement("section");
    sectionElem.innerHTML = "<div class='content'></div>";
    sectionElem.id = "section-group-" + groupId;
    //div
    var HtmlStr = "<div class='messages'><div class='groupLeftContent'></div><div class='groupRightContent'><menu></menu></div></div><div class='write-form'><textarea data-type='group'  data-groupId='";
    HtmlStr += groupId;
    HtmlStr += "' placeholder=' Type your message ' rows='2'></textarea><span class='send'>发送</span></div>";
    sectionElem.firstElementChild.innerHTML = HtmlStr;
    console.log(sectionElem.firstElementChild.lastElementChild.lastElementChild);
    sectionElem.firstElementChild.lastElementChild.lastElementChild.addEventListener("click", SendClick);
    contentChat.appendChild(sectionElem);
    //请求群成员列表;
    $.ajax({
        url: '/data/groupMember?groupId=' + groupId, dataType: 'Text', async: true, success: function (data) {
            appendGroupMember(groupId, data);
            console.log(groupId, data);
        }
    });
    //触发点击事件，修改样式
    EventClick(sectionChatTabs.lastElementChild.firstElementChild);
}
//获取友好时间
function getTime(timestamp) {
    nowTime = new Date();
    beforeTime = new Date(parseInt(timestamp));
    var timestamp2 = nowTime.getTime();
    var timeDef = 0;
    if (nowTime > beforeTime) {
        timeDef = nowTime - beforeTime;
    } else {
        timeDef = beforeTime - nowTime;
    }
    if (timeDef < 60000) return "刚刚";
    else if (60000 <= timeDef && timeDef <= 86400000) {
        return beforeTime.getHours() + ":" + beforeTime.getMinutes() + ":" + beforeTime.getSeconds();
    }
    else {
        return beforeTime.getUTCFullYear() + "/" + (beforeTime.getMonth() + 1) + "/" + beforeTime.getDate() + "  " + beforeTime.getHours() + ":" + beforeTime.getMinutes() + ":" + beforeTime.getSeconds();
    }
}
//聊天记录; data = {fromId:'aaa', toId: 'me', timeStamp:'1429692320401',msgData:'Hello'}
function appendNetPeerMsg(data) {
    var sectionElem = document.getElementById("section-peer-" + data.fromId);
    if (null != sectionElem) {
        var msgList = sectionElem.querySelector(".messages");
        var divElem = document.createElement("div");
        divElem.className = "leftMessage";
        new Date(parseInt(data.timestamp));
        divElem.innerHTML = " <div class='head'><span class='time'>" + getTime(data.timeStamp) + "</span> </div><div class='message leftIcon'>" + data.msgData + "</div>";
        
        msgList.appendChild(divElem);
        //滚动到底部
        msgList.scrollTop = msgList.scrollHeight;
    }
}
function appendLocalPeerMsg(data) {
    var sectionElem = document.getElementById("section-peer-" + data.toId);
   // debugger;
    if (null != sectionElem) {
        var msgList = sectionElem.querySelector(".messages");
        var divElem = document.createElement("div");
        divElem.className = "rightMessage";
        new Date(parseInt(data.timestamp));
        divElem.innerHTML = " <div class='head'><span class='time'>" + getTime(data.timeStamp) + "</span> </div><div class='message rightIcon'>" + data.msgData + "</div>";
        msgList.appendChild(divElem);
        //滚动到底部
        msgList.scrollTop = msgList.scrollHeight;
    }
}
//聊天记录; data = {fromId:'14006', toId: '1', timeStamp:'1429692320401',msgData:'Hello'}
function appendNetGroupMsg(data) {
    if (data.fromId != g_userId) {
        
        var sectionElem = document.getElementById("section-group-" + data.toId);
        if (null != sectionElem) {
            var msgList = sectionElem.querySelector(".groupLeftContent");
            var divElem = document.createElement("div");
            divElem.className = "leftMessage";
            new Date(parseInt(data.timestamp));
            divElem.innerHTML = " <div class='head'><span class='time'>" + getTime(data.timeStamp) + "</span> </div><div class='message leftIcon'>" + data.msgData + "</div>";
            
            msgList.appendChild(divElem);
            //滚动到底部
            msgList.scrollTop = msgList.scrollHeight;
        }
    }
}
//添加自己的记录，var data = {fromId:'14006', toId: '1', timeStamp:'1429692320401',msgData:'Hello'}
function appendLocalGroupMsg(data) {
    var sectionElem = document.getElementById("section-group-" + data.toId);
    if (null != sectionElem) {
        var msgList = sectionElem.querySelector(".groupLeftContent");
        var divElem = document.createElement("div");
        divElem.className = "rightMessage";
        new Date(parseInt(data.timestamp));
        divElem.innerHTML = " <div class='head'><span class='time'>" + getTime(data.timeStamp) + "</span> </div><div class='message rightIcon'>" + data.msgData + "</div>";
        msgList.appendChild(divElem);
        //滚动到底部
        msgList.scrollTop = msgList.scrollHeight;
    }
}

//添加群成员列表
function appendGroupMember(groupId, data) {
    //userId, nickName, image
    var myGroupSection = document.getElementById("section-group-" + groupId);
    var myMenu = myGroupSection.querySelector(".groupRightContent").firstElementChild;
    var infoLen = 3;
    dataObj = JSON.parse(data);
    for (var i = 0; i < dataObj.length / infoLen ; i++) {
        var liElem = document.createElement("li");
        liElem.innerHTML = "<img src='image/aa.jpg'><span class='groupMember' data-userId='" + dataObj[infoLen * i] + "'>" + dataObj[infoLen * i + 1] + "</span>";
        myMenu.appendChild(liElem);
    }
}
//<li><a href="#section-14006" data-type="peer" data-groupId="1" data-userId="14006" class="navIcon icon-friends"><span>好友</span></a>
//<span class="close" ></span>
//</li>
//<section id="section-peer-14006">
//<div class="content">1</div>
//</section>