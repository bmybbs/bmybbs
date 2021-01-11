Page({
	data: {
		code: ""
	},
	onLoad: function (options) {
		this.setData({
			code: options.code
		})
	},
	goBack: function() {
		wx.navigateBack({
			delta: 3
		})
	},
})

