<template>
	<section class="card bmy-dashboard-card">
		<div class="card-header">
			{{ _name }}
		</div>
		<ul class="list-group list-group-flush">
			<li class="list-group-item">Cras justo odio</li>
			<li class="list-group-item">Dapibus ac facilisis in</li>
			<li class="list-group-item">Vestibulum at eros</li>
		</ul>
		<div class="card-footer">
			热门版面：foo bar baz ...
		</div>
	</section>
</template>

<script>
export default {
	data() {
		return { };
	},
	props: {
		_name: String,
	},
}
</script>

