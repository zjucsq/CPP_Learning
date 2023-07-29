Golang、Kotlin、C#、JS、Python等都有协程，市面上的协程有什么本质上的区别？ - readcopy的回答 - 知乎
https://www.zhihu.com/question/502314022/answer/2343470555

https://www.zhihu.com/question/19801131
回调与协程？

https://www.zhihu.com/question/538436096/answer/2537304895
https://zhuanlan.zhihu.com/p/32431200
有栈协程

https://www.zhihu.com/question/538063010
无栈协程

https://blog.csdn.net/github_18974657/article/details/108526591


https://zhuanlan.zhihu.com/p/414506528
co_return --> __context->_promise->return_void(); goto final_suspend_label;
co_yield "hello" --> co_await __context->_promise->yield_value("hello");
 
https://zhuanlan.zhihu.com/p/349710180


实现generator https://zhuanlan.zhihu.com/p/599053058

https://blog.csdn.net/qq_21438461/article/details/131230589

https://zhuanlan.zhihu.com/p/371923001
异步编程