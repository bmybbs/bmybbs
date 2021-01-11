import { BMYClient } from "./BMYClient.js"
import { BMY_EC } from "./BMYConstants.js"

const BMYLogin = function(username, password, toast, login_callback) {
	BMYClient.user_login(username, password).then(response => {
		switch (response.errcode) {
		case BMY_EC.API_RT_SUCCESSFUL:
			login_callback();
			break;

		case 1: // YTHTBBS_USER_NOT_EXIST
			toast.error("用户不存在", { position: "top" });
			break;

		case 2: // YTHTBBS_USER_WRONG_PASSWORD
			toast.warning("密码不正确", { position: "top" });
			break;

		case 4: // YTHTBBS_USER_TOO_FREQUENT
			toast.warning("登录太频繁，请稍候", { position: "top" });
			break;

		case 5: // YTHTBBS_USER_SITE_BAN
		case 6: // YTHTBBS_USER_USER_BAN
		case 7: // YTHTBBS_USER_IN_PRISON
			toast.error("当前用户被禁止登录", { position: "top" });
			break;

		case 8: // YTHTBBS_USER_ONLINE_FULL
			toast.error("当前登录人数已达到系统上限", { position: "top" });
			break;

		case 9: // YTHTBBS_USER_SESSION_ERROR
			break;

		case BMY_EC.API_RT_WRONGPARAM:
		default:
			toast.error("内部错误", { position: "top" });
			break;
		}
	});
}

export { BMYLogin };

