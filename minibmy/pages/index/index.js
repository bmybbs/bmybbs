import { BMYClient } from "../../utils/BMYClient.js"
import { BMY_EC } from "../../utils/BMYConstants.js"
//获取应用实例
const app = getApp()

Page({
	data: {
		motto: 'Hello World',
		userInfo: {},
		hasUserInfo: false,
		canIUse: wx.canIUse('button.open-type.getUserInfo')
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
	onLoad: function () {
		if (app.globalData.userInfo) {
			this.setData({
				userInfo: app.globalData.userInfo,
				hasUserInfo: true
			})
		} else if (this.data.canIUse){
			// 由于 getUserInfo 是网络请求，可能会在 Page.onLoad 之后才返回
			// 所以此处加入 callback 以防止这种情况
			app.userInfoReadyCallback = res => {
				this.setData({
					userInfo: res.userInfo,
					hasUserInfo: true
				})
			}
		} else {
			// 在没有 open-type=getUserInfo 版本的兼容处理
			wx.getUserInfo({
				success: res => {
					app.globalData.userInfo = res.userInfo
					this.setData({
						userInfo: res.userInfo,
						hasUserInfo: true
					})
				}
			})
		}
	},
	getUserInfo: function(e) {
		console.log(e)
		app.globalData.userInfo = e.detail.userInfo
		this.setData({
			userInfo: e.detail.userInfo,
			hasUserInfo: true
		})
	}
})

