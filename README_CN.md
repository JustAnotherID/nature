nature 编程语言

通用开源编程语言与编译器，期望为开发者提供**简洁优雅**的开发体验，能够**简单高效**构建出安全可靠的跨平台软件。


## 特性

- ✓ 简洁、自然、一致性的语法设计
- ✓ 独立的编译系统，不依赖 llvm，支持跨平台编译
- ✓ 完善的类型系统，支持泛型、空值安全、错误处理
- ✓ 自动 GC，具有非常短暂的 STW
- ✓ 模块化与包管理系统 npkg
- ✓ 内置共享栈协程，每秒进行数百万的协程切换
- ✓ 内置 libuv 与协程合作处理 IO 事件循环
- ✓ 内置常用数据结构及标准库实现，并且方便的和 C 语言交互
- ✓ 编辑器 lsp 支持
- ○ 渐进式 GC，能够进行手动内存管理
- ○ 测试 DSL，高效稳定利用 AI 编码
- ○ macho 跨平台连接器，目前缺少 macho 链接器 darwin 无法跨平台编译。
- ○ 协作式调度系统完善
- ○ 跨平台编译支持 wasm、risc64 平台
- ○ 编译为可读的 golang 编程语言

## 项目概况

nature 编程语言已经达到可用版本，语法 API 基本稳定，在 1.0 版本之前不会有大幅的变化，会有小范围的语法改动。

当前版本 nature 源码支持编译的目标平台有
- linux/amd64
- linux/arm64
- darwin/amd64
- darwin/arm64

nature 包含一组测试用例及标准库用来测试语法的可用性，但未经过中大型的项目测试，所以还有较多的 bug。

后续版本的主要工作有
- 标准库完善
- 收集用户反馈
- 项目验证及 bug 修复
- 完善语言特性
- 编译为可读 golang 编程语言以增加 nature 编程语言可用性


官网: [https://nature-lang.org](https://nature-lang.org)

## 安装

从 [releases](https://github.com/nature-lang/nature/releases) 中下载并解压 natrue 安装包。推荐将解压后的 nature 文件夹移动到 `/usr/local/` 下，并将 `/usr/local/nature/bin` 目录加入到系统环境变量。

创建一个 main.n 文件

```js
import fmt

fn main() {
	fmt.printf('hello nature')
}
```

编译并执行

```sh
> nature build main.n && ./main
hello nature
```

---

编辑器 lsp 安装 https://github.com/nature-lang/nls

编码示例 https://github.com/nature-lang/nature/tree/master/tests/features/cases

## 文档

 [https://nature-lang.org/docs/getting-started/hello-world](https://nature-lang.org/docs/getting-started/hello-world)

## 贡献指南

https://nature-lang.org/docs/prologue/contribution-guide

## License

本项目的编译器前端、后端、runtime 等项目源码受 Apache License (Version 2.0) 限制。当前项目中包含的 nature 源码文件(.n) 如标准库等使用 MIT 许可证。