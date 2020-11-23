<template>
	<LoginMobile v-if="is_mobile" />
	<LoginPC v-bind:_loginok="loginok" v-bind:_userid="userid" v-bind:_loginpics="loginpics" v-else />
</template>

<script>
import LoginPC from "@/components/LoginPC.vue";
import LoginMobile from "@/components/LoginMobile.vue";

export default {
	data() {
		return {
			is_mobile: false,
			loginok: false,
			userid: "",
			loginpics: [],
		};
	},
	created() {
		this.check_width();
		window.addEventListener("resize", this.resize_handler);

		fetch("/BMY/loginpics")
			.then(response => response.json())
			.then(response => {
				let that = this,
					arr = response.data.split(";;");

				arr.forEach((el, idx) => {
					let tmp = el.split(";");
					that.loginpics.push({
						index: idx,
						display: (idx == 0),
						img_url: tmp[0],
						img_link: tmp[1]
					});
				});
			});

		fetch("/BMY/user_check")
			.then(response => response.json())
			.then(response => {
				if (response.code == 0) {
					this.loginok = true;
					this.userid  = response.userid;
				}
			});
	},
	unmounted() {
		window.removeEventListener("resize", this.resize_handler);
	},
	components: {
		LoginPC,
		LoginMobile,
	},
	methods: {
		check_width() {
			this.is_mobile = (window.innerWidth <= 600);
		},
		resize_handler() {
			this.check_width();
		}
	},
};
</script>

<style>
</style>

