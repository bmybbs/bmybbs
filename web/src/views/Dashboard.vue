<template>
	<h2>兵马俑导读</h2>

	<h3>美文推荐</h3>
	<DashboardCommend v-bind:_articles="commend" />

	<h3>通知公告</h3>
	<DashboardCommend v-bind:_articles="announce" />
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import DashboardCommend from "@/components/DashboardCommend.vue"

export default {
	data() {
		return {
			announce: [],
			commend: [],
		}
	},
	mounted() {
		const that = this;
		BMYClient.get_announce()
			.then(response => {
				// TODO
				response.articlelist.forEach((item) => that.announce.push(item));
			});
		BMYClient.get_commend()
			.then(response => {
				// TODO
				response.articlelist.forEach((item) => that.commend.push(item));
			});
	},
	components: {
		DashboardCommend,
	},
};
</script>

