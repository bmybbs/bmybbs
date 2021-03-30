<template>
	<div class="container">
		<div class="row">
			<div v-for="section in sections"
				:key="section.id"
				class="col-4"
			>
				<div class="d-flex flex-column align-items-center section-item mb-4" @click="gotoSection(section.id)">
					<div class="bmy-circle-border w-100">
						<div class="ratio ratio-1x1 text-center">
							<div class="fs-1 align-middle text-bmy-blue2">
								<fa :icon="section.icon" class="d-inline-block h-100"/>
							</div>
						</div>
					</div>
					<div class="section-name fs-5 pt-2">
						{{ section.name }}
					</div>
				</div>
			</div>
		</div>
	</div>
</template>

<script>
import { BMYSECSTRS } from "@/lib/BMYConstants.js"

export default {
	data() {
		return {
			sections: BMYSECSTRS,
		}
	},
	methods: {
		gotoSection(id) {
			this.$router.push(`/web/section/${id}`);
		},
	},
}
</script>

<style lang="scss" scoped>
@import "@/assets/gradient-circle-border.scss"
</style>

