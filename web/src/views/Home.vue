<template>
	<div class="d-block d-md-none">
		<LoginMobile v-bind:_loginok="loginok" v-bind:_userid="userid" v-bind:_checked="checked" />
	</div>
	<div class="d-none d-md-block">
		<LoginPC v-bind:_loginok="loginok" v-bind:_userid="userid" v-bind:_checked="checked" v-bind:_loginpics="loginpics" />
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"

const LoginPC = defineAsyncComponent(() => import("@/components/LoginPC.vue"));
const LoginMobile = defineAsyncComponent(() => import("@/components/LoginMobile.vue"));

export default {
	data() {
		return {
			loginok: false,
			userid: "",
			loginpics: [],
			checked: false,
		};
	},
	created() {
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

		BMYClient.user_check().then(response => {
			if (response.code == 0) {
				this.loginok = true;
				this.userid  = response.userid;
			}
			this.checked = true;
		});
	},
	components: {
		LoginPC,
		LoginMobile,
	},
	methods: {
	},
};
</script>

<style>
</style>

