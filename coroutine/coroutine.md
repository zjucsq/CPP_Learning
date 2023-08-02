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

C++协程解释
https://zhuanlan.zhihu.com/p/349210290
https://zhuanlan.zhihu.com/p/349710180

不错的解释
https://zhuanlan.zhihu.com/p/59178345

如果要实现形如co_await blabla;的协程调用格式, blabla就必须实现Awaitable。co_await是一个新的运算符。Awaitable主要有3个函数：
1. await_ready：返回Awaitable实例是否已经ready。协程开始会调用此函数，如果返回true，表示你想得到的结果已经得到了，协程不需要执行了。所以大部分情况这个函数的实现是要return false。
2. await_suspend：挂起awaitable。该函数会传入一个coroutine_handle类型的参数。这是一个由编译器生成的变量。在此函数中调用handle.resume()，就可以恢复协程。
3. await_resume：当协程重新运行时，会调用该函数。这个函数的返回值就是co_await运算符的返回值。