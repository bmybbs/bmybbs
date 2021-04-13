import { BMYClient } from "../../utils/BMYClient.js"
import { BMY_EC } from "../../utils/BMYConstants.js"

Component({
	properties: {
		is_fav: { type: Number, value: 0 },
		size: { type: Number, value: 20 },
		boardname: { type: String, value: "" },
	},
	data: {
	},
	methods: {
		onToggle() {
			if (this.data.is_fav == 1) {
				BMYClient.fav_del(this.data.boardname).then(response => {
					if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
						this.setData({ is_fav: false });
					} else if (response.errcode == BMY_EC.API_RT_NOTLOGGEDIN) {
						wx.showToast({
							title: "请先登录",
							icon: "error",
							duration: 2000
						});
					}
				});
			} else {
				BMYClient.fav_add(this.data.boardname).then(response => {
					const msg = {
						title: "",
						icon: "error",
						duration: 2000
					};

					switch (response.errcode) {
						case BMY_EC.API_RT_SUCCESSFUL:
						case BMY_EC.API_RT_ALRDYINRCD:
							this.setData({ is_fav: true });
							return;

						case BMY_EC.API_RT_NOTLOGGEDIN:
							msg.title = "请重新登录";
							break;
						case BMY_EC.API_RT_REACHMAXRCD:
							msg.title = "收藏夹已满";
							break;
						case BMY_EC.API_RT_NOSUCHBRD:
						case BMY_EC.API_RT_FBDNUSER:
							msg.title = "版面不存在";
							break;
						default:
							msg.tilte = "未知错误";
					}

					wx.showToast(msg);
				});
			}
		},
	},
})

