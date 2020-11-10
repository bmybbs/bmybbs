<template>
	<div class="container-fluid">
		<div class="row">
			<div class="bmy-content">
				<ul class="index-nav float-right">
					<li><a>匿名登录</a></li>
					<li><a>telnet登录</a></li>
					<li><a>CTerm工具下载</a></li>
					<li><a>新用户注册</a></li>
				</ul>
				<div id="pic-container">
					<div class="slides" v-for="item in loginpics" :key="item.img_url" v-bind:style="{ display: item.display ? 'block' : 'none' }">
						<a v-bind:href="item.img_link"><img v-bind:src="item.img_url"></a>
					</div>

					<div class="prev" v-on:click="showPrev">&#10094;</div>
					<div class="next" v-on:click="showNext">&#10095;</div>
				</div>
				<div id="pic-nav">
					<div v-for="item in loginpics" :key="item.img_url" v-bind:class="{ active: item.display }" v-on:click="showSlides(item.index)"></div>
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
			slideIndex: 0,
			loginpics: [],
			loginok: false,
			checked: false
		}
	},
	created() {
	},
	mounted() {
		let that = this;
		fetch("/BMY/loginpics")
			.then(response => response.json())
			.then(response => {
				let str = response.data,
					arr = str.split(";;");

				arr.forEach((el, idx) => {
					let tmp = el.split(";");
					console.log(idx);
					that.loginpics.push({
						index: idx,
						display: (idx == 0),
						img_url: tmp[0],
						img_link: tmp[1]
					});
				});
			});
	},
	methods: {
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

		showPrev() {
			this.showSlides(this.slideIndex - 1);
		},

		showNext() {
			this.showSlides(this.slideIndex + 1);
		}
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

.bmy-content ul.index-nav {
	list-style: none;
	color: #aaa;
	font-size: 12px;
	margin-bottom: 5px;
}

.bmy-content ul.index-nav li {
	display: inline;
}

.bmy-content ul.index-nav li:after {
	content: "/";
	padding-left: .3em;
	padding-right: .3em;
	color: #000;
}

.bmy-content ul.index-nav li:last-child:after {
	content: "";
	padding: none;
}

#pic-container {
	display: relative;
}

#pic-container img {
	box-shadow: 0 10px 20px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
}

.prev,
.next {
	cursor: pointer;
	position: absolute;
	top: 50%;
	width: auto;
	padding: 16px;
	margin-top: -20px;
	color: white;
	font-weight: bold;
	font-size: 20px;
	border-radius: 0 3px 3px 0;
	user-select: none;
	background: rgba(0, 0, 0, 0.1);
	-webkit-user-select: none;
}

.next {
	right: 0;
	border-radius: 3px 0 0 3px;
}

.prev:hover, .next:hover {
	background-color: rgba(0, 0, 0, 0.8);
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
</style>

