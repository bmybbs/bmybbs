<template>
	<div class="card">
		<div class="card-header">
			好友列表
		</div>

		<ul class="list-group list-group-flush">
			<li class="list-group-item"
				v-for="user in friends"
				v-bind:key="user.username">
				<span class="online" v-if="user.online"></span>
				<span class="offline" v-else></span>
				{{ user.username }}
			</li>
		</ul>
	</div>
</template>

<script>
export default {
	data() {
		return {
			friends: [
				{ username: "foo", online: true },
				{ username: "bar", online: false },
				{ username: "baz", online: false },
				{ username: "tee", online: false },
			],
		};
	}
}
</script>

<style scoped>
span {
	display: inline-block;
	width: 12px;
	height: 12px;
	border-radius: 50%;
	margin-right: 4px;
}

span.online {
	background-color: green;
}
</style>

