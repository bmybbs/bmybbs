<template>
	<LoginMobile v-if="is_mobile" />
	<Login v-else />
</template>

<script>
import Login from "@/components/Login.vue";
import LoginMobile from "@/components/LoginMobile.vue";

export default {
	data() {
		return {
			is_mobile: false,
		};
	},
	created() {
		this.check_width();
		window.addEventListener("resize", this.resize_handler);
	},
	unmounted() {
		window.removeEventListener("resize", this.resize_handler);
	},
	components: {
		Login,
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

