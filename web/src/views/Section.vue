<template>
	<div class="container">
		<div class="row">
			<div v-for="section in sections"
				:key="section.id"
				class="col-4"
			>
				<div class="d-flex flex-column align-items-center section-item mb-4" @click="gotoSection(section.id)">
					<div class="border border-dark border-5 rounded-circle ratio ratio-1x1 section-icon-border text-center">
						<div class="section-icon align-middle">
							<fa :icon="section.icon" class="d-inline-block h-100"/>
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

<style scoped>
.section-icon {
	font-size: 2.5rem;
}
</style>

