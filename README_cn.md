# beast_utils

## 动机

bottle是一个符合WSGI接口规范的web框架，它的优点在于：

* 功能上符合"单一职责"原则。 只做一件事(request路由)，不包含其它web功能， 如：数据库映射、模板渲染引擎、管理页面、权限控制等；
* 路由处理上符合"开闭原则"(允许添加、谢绝修改)。新增路由处理时，只需新增视图函数，无需修改其它视图文件。

然而，它是基于单线程的，特别是在单个用户请求处理完成并断开连接前，不接受其他用户的连接。在实际应用上更多的是采用"nginx + cherrypy + bottle"方案:

* nginx做前端代理，接收多客户并发请求，提供负载均衡实现；
* cherrypy提供多线程支持；
* bottle提供单线程请求处理。

**有没有更好的替代方案呢？**

直到我们找到boost.beast：它是一个高性能的C++ web异步响应实现, 其示例文件[advanced_server_flex.cpp](https://www.boost.org/doc/libs/1_77_0/libs/beast/example/advanced/server-flex/advanced_server_flex.cpp)中的实现, 同时具有以下优点:

* 异步响应(boost.asio)；
* 超时处理；
* 多线程支持；
* 单一端口多协议: HTTP、HTTPS、WS, WSS。

采用"boost.beast + bottle“的组合，正好各取所长：

* "beast_utils"共享库(beast_utils.dll、beast_utils.so、...)实现服务器消息循环，异步接收用户并发请求, 并将请求委托给相应的回调函数;
* "bottle"提供回调函数实现, 提供单一请求处理实现;
* "python.ctypes"作为glue, 将python实现的回调函数注册到"beast_utils"共享库中。

## 构建

参考[构建说明](BUILD.md)
