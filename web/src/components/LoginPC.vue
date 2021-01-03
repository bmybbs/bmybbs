<template>
	<div class="container-fluid">
		<div class="row">
			<div class="bmy-content">
				<ol class="breadcrumb float-end index-nav">
					<li class="breadcrumb-item"><a>匿名登录</a></li>
					<li class="breadcrumb-item"><a>telnet登录</a></li>
					<li class="breadcrumb-item"><a>CTerm工具下载</a></li>
					<li class="breadcrumb-item"><a>新用户注册</a></li>
				</ol>
				<div id="pic-container" v-on:mouseEnter="stopAnimation" v-on:mouseleave="startAnimation">
					<div class="slides" v-for="item in loginpics" :key="item.img_url" v-bind:style="{ display: item.display ? 'block' : 'none' }">
						<a v-bind:href="item.img_link"><img v-bind:src="item.img_url"></a>
					</div>
				</div>
				<div id="pic-nav" v-on:mouseEnter="stopAnimation" v-on:mouseleave="startAnimation">
					<div v-for="item in loginpics" :key="item.img_url" v-bind:class="{ active: item.display }" v-on:click="showSlides(item.index)"></div>
				</div>

				<!-- welback/loginform switch begin -->
				<div v-if="_checked">
					<div id="welback" class="row justify-content-center" v-if="_loginok">
						<div class="col-auto text-end">
							欢迎回来 {{ _userid }}
						</div>
						<div class="col-auto btn-group">
							<button type="button" class="btn btn-sm btn-primary">进入旧版</button>
							<button type="button" class="btn btn-sm btn-primary">进入新版</button>
						</div>
					</div>
					<form ref="form" id="login-form" class="row" action="/BMY/bbslogin" method="post" v-else>
						<div class="col-sm-4">
							<input name="id" type="text" class="form-control form-control-sm" placeholder="账号" v-model="username">
						</div>
						<div class="col-sm-4">
							<input name="pw" type="password" class="form-control form-control-sm" placeholder="密码" v-model="password">
						</div>
						<div class="col-4 btn-group">
							<button type="button" class="btn btn-sm btn-primary" v-on:click="post_form">登录旧版</button>
							<button type="button" class="btn btn-sm btn-primary">登录新版</button>
							<button type="button" class="btn btn-sm btn-primary">忘记密码</button>
						</div>
					</form>
				</div>
				<!-- welback/loginform switch end -->

				<div id="footer">
					陕ICP备 05001571号<br>开发维护：西安交通大学网络中心 BBS程序组
				</div>
			</div>
		</div>
	</div>
</template>

<script>
export default {
	name: "Login",
	data() {
		return {
			userid: this._userid,
			intervalID: null,
			username: "",
			password: "",
			slideIndex: 0,
			loginpics: this._loginpics,
			loginok: this._loginok,
		}
	},
	created() {
	},
	mounted() {
		this.startAnimation();
	},
	unmounted() {
		this.stopAnimation();
	},
	props: {
		_checked: Boolean,
		_loginok: Boolean,
		_userid: String,
		_loginpics: Array,
	},
	methods: {
		post_form() {
			this.$refs.form.submit();
		},
		showSlides(n) {
			if (this.slideIndex == n)
				return;

			if (n > this.loginpics.length - 1) {
				this.slideIndex = 0;
			} else if (n < 0) {
				this.slideIndex = this.loginpics.length - 1;
			} else {
				this.slideIndex = n;
			}

			for (let i = 0; i < this.loginpics.length; i++) {
				this.loginpics[i].display = this.slideIndex == i;
			}
		},
		startAnimation() {
			var that = this;
			if (this.intervalID == null) {
				this.intervalID = setInterval(function() {
					that.showSlides(that.slideIndex + 1);
				}, 5000);
			}
		},
		stopAnimation() {
			if (this.intervalID) {
				clearInterval(this.intervalID);
				this.intervalID = null;
			}
		},
	}
};
</script>

<style scoped>
.container-fluid {
	padding-top: 47px;
}

.bmy-content {
	width: 794px;
	margin: auto;
}

.bmy-content ol.index-nav {
	color: #999;
	font-size: 14px;
	margin-bottom: 5px;
}

#pic-container {
	display: relative;
}

#pic-container img {
	box-shadow: 0 10px 20px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
}

#pic-nav {
	width: 120px;
	margin: 0 auto;
	padding-top: 16px;
}

#pic-nav div {
	cursor: pointer;
	border: 1px solid #FF6600;
	display: inline-block;
	width: 16px;
	height: 16px;
	margin-right: 10px;
	border-radius: 50%;
}

#pic-nav div:last-child {
	margin-right: 0px;
}

#pic-nav div.active {
	background-color: #FF6600;
}

#footer {
	text-align: center;
	font-size: 12px;
	color: #222222;
}
</style>

