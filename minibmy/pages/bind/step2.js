import { BMYClient } from "../../utils/BMYClient.js";
import { BMY_EC } from "../../utils/BMYConstants.js"

Page({
	data: {
		tfakey: "",
	},
	submitKey: function(tfakey) {
		wx.login({
			success: res => {
				BMYClient.oauth_get_code(res.code, tfakey).then(response => {
					if (response.errcode == BMY_EC.API_RT_2FA_INTERNAL) {
						wx.showToast({
							title: "已过期或有误",
							icon: "error",
							duration: 2000
						})
					} else if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
						wx.navigateTo({
							url: `step3?code=${response.code}`
						})
					}
				})
			}
		})
	},
	scanQRCode: function() {
		const that = this;
		wx.scanCode({
			onlyFromCamera: true,
			scanType: "qrCode",
			success: function(res) {
				that.submitKey(res.result);
			}
		})
	},
	getInput: function(e) {
		this.setData({
			tfakey: e.detail.value
		})
	},
	inputKey: function() {
		this.submitKey(this.data.tfakey);
	},
})

