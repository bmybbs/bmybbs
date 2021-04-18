import { BMY_EC } from "./BMYConstants.js"

const errorMsgMap = new Map();
errorMsgMap.set(BMY_EC.API_RT_NOTLOGGEDIN,  "请先登录");
errorMsgMap.set(BMY_EC.API_RT_WRONGPARAM,   "参数错误");
errorMsgMap.set(BMY_EC.API_RT_ATCLNOTITLE,  "缺少标题");
errorMsgMap.set(BMY_EC.API_RT_NOSUCHBRD,    "版面不存在");
errorMsgMap.set(BMY_EC.API_RT_NOSUCHUSER,   "用户不存在");
errorMsgMap.set(BMY_EC.API_RT_NOBRDPPERM,   "您被禁止发帖");
errorMsgMap.set(BMY_EC.API_RT_CNTMAPBRDIR,  "版面内部错误");
errorMsgMap.set(BMY_EC.API_RT_ATCLFBDREPLY, "本文不可回复");
errorMsgMap.set(BMY_EC.API_RT_USERLOCKFAIL, "用户内部错误");
errorMsgMap.set(BMY_EC.API_RT_WRONGTOKEN,   "请再试一次");
errorMsgMap.set(BMY_EC.API_RT_FBDGSTIP,     "GUEST 无法发帖");
errorMsgMap.set(BMY_EC.API_RT_ATCLINNERR,   "文章内部错误");
errorMsgMap.set(BMY_EC.API_RT_ATCL1984,     "不适宜发表");

export const getErrorMessage = (errcode) => {
	return errorMsgMap.has(errcode) ? errorMsgMap.get(errcode) : "未知错误";
}

