### 目录结构 

```
    omi   
    | 
    | - app
    |     |  
    |     | - core
    |          |
    |          | - handler
    |                |
    |                | - messages_handler.go    => message处理
    |                |
    |                | - xxxx_handler.go        => 其他业务的处理 
    |                |
    |          | - middleware
    |                |
    |                | - auth.go                => 身份校验
    |                |
    |                | - language.go            => 更新用户的Accept-Language
    |                |
    |                | - location.go            => 更新用户的地理位置信息
    |                |
    |          | - routers
    |                |
    |                | - router.go              => 注册全部路由到gin
    |                | 
    |                | - message_route.go       => message相关的路由
    |
    | - config
    |     |
    |     | - config.go                         => 定义各种配置的结构体
    |
    | - event
          |
          | - event.go                          => 定义Event结构体
          | 
          | - event_manager.go                  => Event写channel、消费Event
          |
          | - handler
               |
               | - xxxx_eventhandler.go         => Event处理函数  
```

<br>

### 服务启动过程

1. 初始化kafka、redis、db、slog等的配置（/etc/omi/xxx.json）
2. gin路由注册、中间件注册

<br>

### 路由

```
// vendor/gitlab.p1staff.com/backend/tantan-backend-common/http/server.go
type Router interface {
	Route(*gin.Engine)
}
```
----

```
// app/core/routers/message_route.go
func init() {
	Registered(&MessageRouter{})
}

type MessageRouter struct{}

func (s *MessageRouter) Route(g *gin.Engine) {
	g.GET(path, handler)
}
```

----

```
// app/core/routers/routers.go
type CoreServerRouterGroup struct{}
subRoutes   []server.Router

func (s *CoreServerRouterGroup) Route(g *gin.Engine) {
	if len(subRoutes) == 0 {
		return
	}
	lock.RLock()
	defer lock.RUnlock()
	if len(middlewares) > 0 {
		g.Use(middlewares...)
	}
	for _, r := range subRoutes {
		r.Route(g)
	}
}

func Registered(r server.Router) {
	lock.Lock()
	defer lock.Unlock()
	if Existed(r) {
		return
	}
	subRoutes = append(subRoutes, r)
}
```

<br>

### 中间件

```
1. 身份校验，以及获取用户手机系统版本及app版本号等信息
2. 更新用户的AppLanguage，利用该字段，为后续的多语言相关的作准备
3. 更新用户的地理位置信息
```

<br>

### 数据库操作

```
type dbGroup struct {
	physicalShardId int

	master      pgDBWrapper    // db connect for write
	readOnly    []pgDBWrapper  // db connect for read, round-robin访问
	abTestSlave []pgDBWrapper  // slaves for AB testing

	roundRobin     int64
	roundRobinLock sync.Mutex

	roundRobinAbTest int64     // round robin LB for ab testing slaves
}
```

**输入**：shard_id, op(read or write)，sql模板, 查询参数 

**处理**：

1. 生成shard_id, message部分的shard_id = (user_id + other_user_id) % mod 
2. shardMapByUser[shard_id]得到dbGroup
3. 如果op等于write，返回dbGroup.master。如果op等于read，根据round-robin返回其中的某一个dbGroup.readOnly[i]
4. 根据shard_id、查询参数拼接sql
5. 执行sql

**输出**：结果集

<br>

### 事件处理

```
type Manager struct {
        eventQueue      chan *Event
        gracefulServer  util.GracefulMonitor
        handlers        []Handler
        eventCounterVec *prometheus.CounterVec
}
```
- Publish(event *Event) 发送事件 
- readQueue() 读事件，遍历handlers处理事件

<br>

### Message发送流程  

1. Message格式
```
// 初始的Message结构, external.Message
type Message struct {
    	Id          string            `json:"id"`           // 是否自增
    	Owner       IdType            `json:"owner"`        // 发送者
    	OtherUser   IdType            `json:"otherUser"`    // 接收者
    	Accessory   *Accessory        `json:"accessory"`    // 贴纸
    	Reference   *MessageReference `json:"reference"`    // 未使用，朋友圈相关的
    	Location    *MessageLocation  `json:"location"`     // 位置信息
    	Media       []UserMedia       `json:"media"`        // 图片音视频等
    	Recalled    bool              `json:"recalled"`     // 消息撤回
    	Value       string            `json:"value"`        // 文字
    	CreatedTime Iso8601Time       `json:"createdTime"`  // 创建时间 
    	SentFrom    string            `json:"sentFrom"`     // sentFrom
    	Type        string            `json:"type"`         // message
    	ContentType string            `json:"contentType"`  // 内容类型, audio，sticker等
    
    	StickerLangInfo *StickerMessageLanguageSupport `json:"-"`  // stickers.name.xxxx
}

// 初始Message内部子结构展开，主要展开Accessory，Reference字段, inner.Message   
type Message struct {
    	Id              string
    	UserId          string
    	OtherUserId     string
    	StickerId       string
    	QuestionId      string
    	QuizQuestionId  string
    	QuizAnswerId    string
    	IntimacyLevelId string
    	ReferenceId     string
    	MomentId        string
    	AnswerId        string
    	Location        *MessageLocation
    	Media           []UserMedia
    	Value           string
    	Recalled        bool
    	CreatedTime     time.Time
    	Status          string
    	SentFrom        string
    	Type            string
    	MomentCommentId string
    	IsInspired      bool   // flag for brand inspired comment
    	ContentType     string // identify the message content
    
    	StickerLangInfo *StickerMessageLanguageSupport //表情多语言支持信息
}

// db对应的Message, database.Message
type Message struct {
    	Id          string `pg:"id"`
    	UserId      string `pg:"user_id"`
    	OtherUserId string `pg:"other_user_id"`
    	StickerId   string `pg:"sticker_id"`
    	QuestionId  string `pg:"question_id"`
    	ReferenceId string `pg:"reference_id"`
    	MomentId    string `pg:"moment_id"`
    	Value       string `pg:"value"` // 除普通文字外，还会保存QuizQuestionId, QuizAnswerId, IntimacyLevelId, StickerLangInfo 
    	Recalled    bool   `pg:"recalled"`
    	CreatedTime Time   `pg:"created_time"`
    	SentFrom    string `pg:"sent_from"`
    	Location    string `pg:"location"`
    	Status      string `pg:"status"`
    	ContentType string `pg:"content_type"`
}


```

2. 创建消息，url为 POST ```/users/me/conersations/:otherUid/messages``` 

```
graph TD
A[校验conversation] --> B[解析http body, 初始化Message结构体]
B --> C[如果ContentType是sticker则获取表情名称为后续多语言做准备]
C --> D[校验, 如果男性用户在答完题后未得到对方确认时不能发送消息]
D --> E[初始Message结构体转换成适合db保存的格式]
E --> F[消息存入messages表, 返回结果封装成envelope]
F --> G[更新conversations表的read_until_message_id, message_ids, message_count, unread_messages等]
G --> H[往envelope填充Pagination]
H --> I[往envelope填充统计数据, 包括好友数量, 未读消息数量, 喜欢人数限制, 超级喜欢人数限制等]
I --> J[envelope里边贴纸的一些多语言转换]
J --> K[返回envelope给客户端]
```

3. 消息创建成功后的Event处理
- 更新亲密度
    1. 根据消息类型计算此次亲密度的增量increment
    2. 历史的亲密度值 + increment，得到新的亲密度值和对应的亲密度等级
    3. 更新亲密度值和等级
    4. 如果等级提升，创建一条message记录新的level值，给双方发送静默push
- QuizHeader
    1. 系统自动发的，忽略
- QuizQuestion
    1. 系统自动发的，忽略
- QuizAnswer
    1. 如果男方答完了所有的题目，创建一条message记录答题结束，向女方发送push，请她选择是否通过, 同时向男方发送push，通知conversation状态变化
    2. 如果男方未答完题目，更新已经回答问题数，过滤已经答过的题目和题型，创建一个新的题目保存到message表。给双方发送静默push
- 发送消息创建成功的push

4. 查询消息，url为 GET ``` /users/me/conersations/:otherUid/messages ```
```
graph TD
A[校验conversation] --> B[如果otherUserId非聊天小助手, 做合法性检查]
B --> C[根据查询参数查询messages]
C --> D[返回结果封装成envelope]
D --> E[更新conversations表的read_until_message_id, message_ids, message_count, unread_messages等]
E --> F[往envelope填充Pagination]
F --> G[往envelope填充统计数据, 包括好友数量, 未读消息数量, 喜欢人数限制, 超级喜欢人数限制等]
G --> H[envelope里边贴纸的一些多语言转换]
H --> I[返回envelope给客户端]
```

5. 撤回消息，url为 POST ``` /users/me/conversations/1276/messages/:msgId?method=patch ```
    1. 更新消息的Recalled字段
    2. self.pushClient.PushMessage(&message) 发送push
