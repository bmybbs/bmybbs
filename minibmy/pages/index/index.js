import { BMYClient } from "../../utils/BMYClient.js"
import { BMY_EC } from "../../utils/BMYConstants.js"

Page({
	data: {
	},
	//事件处理函数
	bindGuest: function() {
		wx.switchTab({
			url: '../dashboard/dashboard'
		})
	},
	bindLogin: function() {
		wx.login({
			success: res => {
				BMYClient.oauth_login(res.code).then(response => {
					if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
						wx.switchTab({
							url: '../dashboard/dashboard',
						})
					} else if (response.errcode == BMY_EC.API_RT_WXAPIERROR) {
						wx.showToast({
							title: "微信服务错误",
							icon: "error",
							duration: 2000
						})
					} else if (response.errcode == BMY_EC.API_RT_NOOPENID) {
						wx.showModal({
							title: "提示",
							content: "当前账户没有关联 BMY 账户，是否立即关联？",
							cancelText: "游客登录",
							confirmText: "立即关联",
							success: function(res) {
								if (res.confirm) {
									wx.navigateTo({
										url: "../bind/step1"
									})
								} else {
									wx.switchTab({
										url: '../dashboard/dashboard'
									})
								}
							},
						})
					}
				})
			}
		})
	},
})

