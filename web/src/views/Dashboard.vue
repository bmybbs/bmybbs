<template>
	<div class="container-fluid">
		<h2>兵马俑导读</h2>
		<div class="row">
			<div class="col-md-12 col-lg-9">

				<DashboardCommend v-bind:_name="'美文推荐'" v-bind:_articles="commend" />

				<DashboardCommend v-bind:_name="'通知公告'" v-bind:_articles="announce" />

				<DashboardCommend v-bind:_name="'今日十大'" v-bind:_articles="top10" />

				<DashboardSection v-for="section in sections" v-bind:key="section.name" v-bind:_name="section.name" v-bind:_secstr="section.id" />
			</div>
			<aside class="col-md-12 col-lg-3">
				<DashboardAsideAd />
				<DashboardAsideRecommend />
				<DashboardAsideAbout />
			</aside>
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import { BMYSECSTRS } from "@/lib/BMYConstants.js"
import DashboardCommend from "@/components/DashboardCommend.vue"
import DashboardAsideAd from "@/components/DashboardAsideAd.vue"
import DashboardAsideRecommend from "@/components/DashboardAsideRecommend.vue"
import DashboardAsideAbout from "@/components/DashboardAsideAbout.vue"
import DashboardSection from "@/components/DashboardSection.vue"

export default {
	data() {
		return {
			announce: [],
			commend: [],
			top10: [],
			sections: BMYSECSTRS,
		}
	},
	mounted() {
		BMYClient.get_announce().then(response => {
			this.announce = response.articlelist;
		});
		BMYClient.get_commend().then(response => {
			this.commend = response.articlelist;
		});
		BMYClient.get_top10().then(response => {
			this.top10 = response.articlelist;
		});
	},
	components: {
		DashboardCommend,
		DashboardAsideAd,
		DashboardAsideRecommend,
		DashboardAsideAbout,
		DashboardSection,
	},
};
</script>

<style>
.bmy-dashboard-card {
	margin-bottom: 30px;
}
</style>

