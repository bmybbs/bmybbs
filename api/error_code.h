#ifndef __ERROR_CODE_H
#define __ERROR_CODE_H

enum api_error_code {
	API_RT_SUCCESSFUL 	= 0, 		///< 成功调用
	API_RT_NOEMTSESS	= 1,		///< 系统用户已满
	API_RT_CNTLGOUTGST	= 2,		///< 不能注销guest用户
	API_RT_NOTOP10FILE	= 3,		///< 没有十大文件
	API_RT_XMLFMTERROR	= 4,		///< 十大、分区热门话题文件格式有误
	API_RT_NOSUCHFILE	= 5,		///< 没有找到对应文件，谨慎使用此错误码
	API_RT_NOCMMNDFILE	= 6,		///< 没有找到美文、通知的文件
	API_RT_NOGDBRDFILE	= 7,		///< 没有用户的收藏夹文件
	API_RT_CNTMAPBRDIR	= 8,		///< 不能 MMAP 版面 .DIR 文件
	API_RT_ATCLINNERR	= 9,		///< 发文遇到内部错误
	API_RT_WRONGACTIVE	= 10,		///< 激活码有误
	API_RT_NOBRDTPFILE	= 11,		///< 没有版面置顶文件
	API_RT_NOTENGMEM	= 12,		///< 没有足够的内存
	API_RT_MAILINNERR	= 13,		///< 发信遇到内部错误
	API_RT_FILEERROR    = 14,       ///< 文件操作相关的错误
	API_RT_CNTMKDIR     = 15,       ///< 不能创建文件夹
	API_RT_INVALIDHST   = 16,       ///< 不正确的域名
	API_RT_WRONGPARAM	= 1000,		///< 接口参数错误
	API_RT_WRONGSESS	= 1001,		///< 错误的session
	API_RT_NOTLOGGEDIN	= 1002,		///< 没有登录
	API_RT_FUNCNOTIMPL	= 1003,		///< 功能未实现
	API_RT_WRONGMETHOD	= 1004,		///< 错误的 HTTP 方法
	API_RT_WXAPIERROR   = 1005,     ///< 微信服务调用错误
	API_RT_NOTEMPLATE	= 1100,		///< 没有模板
	API_RT_NOSUCHUSER 	= 100000, 	///< 没有此用户
	API_RT_SITEFBDIP	= 100001,	///< 站点禁用IP
	API_RT_FORBIDDENIP	= 100002,	///< 用户禁用IP
	API_RT_ERRORPWD		= 100003,	///< 用户密码错误
	API_RT_FBDNUSER		= 100004,	///< 用户没有登录权限
	API_RT_INVSESSID	= 100005,	///< 用户session已失效
	API_RT_WRONGTOKEN	= 100006,	///< 错误的用户 token
	API_RT_USEREXSITED	= 100007,	///< 用户已存在
	API_RT_FBDUSERNAME	= 100008,	///< 非法的用户名
	API_RT_REACHMAXRCD	= 100009,	///< 达到最大记录数
	API_RT_ALRDYINRCD	= 100010,	///< 已存在记录中
	API_RT_NOTINRCD		= 100011,	///< 不存在记录中
	API_RT_HASOPENID    = 100012,   ///< 已关联了 openid
	API_RT_2FA_INTERNAL = 100013,   ///< 2fa 创建失败，检查日志
	API_RT_2FA_INVALID  = 100014,   ///< 2fa 不存在或者无效
	API_RT_NOOPENID     = 100015,   ///< 没有关联 openid
	API_RT_USERLOCKFAIL = 100016,   ///< 无法创建用户锁
	API_RT_NOSUCHBRD	= 110000,	///< 没有此版面
	API_RT_NOBRDRPERM	= 110001,	///< 没有该版面的阅读权限
	API_RT_EMPTYBRD		= 110002,	///< 版面没有文章
	API_RT_NOBRDPPERM	= 110003,	///< 没有该版面的发文权限
	API_RT_FBDGSTPIP	= 110004,	///< 禁止 guest 发帖 IP
	API_RT_NOSUCHATCL	= 120000,	///< 没有这篇文章
	API_RT_ATCLDELETED	= 120001,	///< 文章已被删除
	API_RT_ATCLNOTITLE	= 120002,	///< 文章缺少标题
	API_RT_ATCLFBDREPLY	= 120003,	///< 文章禁止回复
	API_RT_ATCL1984     = 120004,   ///< ....
	API_RT_MAILDIRERR	= 130000,	///< 用户邮箱索引错误
	API_RT_MAILEMPTY	= 130001,	///< 用户没有邮件
	API_RT_MAILATTERR	= 130002,	///< 邮件附件错误
	API_RT_MAILNOPPERM	= 130003,	///< 用户没有发邮件的权限
	API_RT_MAILFULL		= 130004,	///< 邮箱已满
	API_RT_INUSERBLIST	= 130005,	///< 在对方黑名单中
	API_RT_ATTINNERR	= 140000,	///< 附件区内部错误（例如文件名）
	API_RT_ATTNOSPACE	= 140001,	///< 附件区没有足够控件
	API_RT_ATTTOOBIG	= 140002,	///< 附件过大
	API_RT_NOMOREFEED   = 150001,   ///< 没有数据
};

#endif
