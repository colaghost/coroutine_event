coroutine_event
===============

在libevent的基础上提供同步的接口，在获得异步效率的同时提供更方便的编程方式，即提供基于协程的并发模型。  

##green化

将IO对象进行改造以能和协程进行配合。在某种意义上，协程与线程的关系类似于线程与进程的关系，你可以将协程理解成用户态线程。目前的IO操作都可能会导致整个线程的挂起，但是我们只希望挂起当前执行的协程，因此需要将IO对象进行改造，让其只会导致挂起当前的协程，而不是整个线程，这里的改造称为“green化”，这个名字来自于python下的一个协程库--greenlet。目前提供的green化的io对象包括：  

* tcp socket
* file descriptor
* timer(定时器，待支持)
* signal(信号，待支持)

##chan：协程间通信

每个协程是一个独立的执行单元，为了能够方便协程之前的通信/同步，coroutine_event提供了chan这种机制。它本质上类似于一个阻塞消息队列，但是它不一定FIFO。它支持在多个线程里面的多个协程之间通信，但不同的线程写数据到同一个chan时，需要有各自的peer，而peer不能跨线程访问。

##多核

一般在多个线程里面使用libevent都是通过一个线程一个event_base，即一个线程一个事件循环。coroutine_event通过一个线程配一个coroutine_base来支持多线程，coroutine_spawn[_with_fd]需要绑定到自己线程独有的coroutine_base上去。

##安装
./configure --prefix=[安装路径] --with-libevent=[libevent安装路径]  
make && make install

##测试例子
test目录下有测试源代码
