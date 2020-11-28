<template>
	<div class="container-fluid">
		<h2>好友动态</h2>
		<div class="row">
			<div class="col-md-12 col-lg-9">
				<div class="card">
					<ul class="list-group list-group-flush">
						<li class="list-group-item"
							v-for="activity in activities"
							v-bind:key="activity.id">
							<span class="time">
								{{ dayjs(activity.timestamp * 1000).fromNow() }}
							</span>
							<span class="username">
								{{ activity.username }}
							</span>
							<span v-if="activity.type == 'POST'">在 {{ activity.board }} 发表了 {{ activity.title }}</span>
							<span v-else-if="activity.type == 'REPLY'">在 {{ activity.board }} / {{ activity.title }} 一文中回复了 {{ activity.author }}</span>
							<span v-else-if="activity.type == 'BOOKMARK'">收藏了 {{ activity.board }} / {{ activity.title }}，作者 {{ activity.author }}</span>
							<span v-else-if="activity.type == 'FOLLOW'">将 {{ activity.author }} 添加到好友列表</span>
							<span v-else-if="activity.type == 'SUBSCRIPT'">订阅了版面 {{ activity.board }}</span>
							<span v-else></span>
						</li>
					</ul>
				</div>
			</div>
			<div class="col-lg-3">
				<FriendsCard />
			</div>
		</div>
	</div>
</template>

<script>
import FriendsCard from "@/components/FriendsCard.vue"
import dayjs from "dayjs"
import relativeTime from "dayjs/plugin/relativeTime"
import "dayjs/locale/zh-cn"

dayjs.locale("zh-cn");
dayjs.extend(relativeTime);

export default {
	data() {
		return {
			dayjs,
			activities: [{
				id: 1,
				username: "foo",
				type: "POST",
				title: "如何使用 xyz",
				board: "XJTUnews",
				timestamp: 1606534804,
			}, {
				id: 2,
				username: "bar",
				type: "REPLY",
				author: "foo",
				title: "今天吃了啥",
				board: "XJTUnews",
				timestamp: 1606534604,
			}, {
				id: 3,
				username: "baz",
				type: "BOOKMARK",
				author: "bar",
				title: "论文写作要点",
				board: "thesis",
				timestamp: 1606531804,
			}, {
				id: 4,
				username: "baz",
				type: "FOLLOW",
				author: "foo",
				timestamp: 1606524804,
			}, {
				id: 5,
				username: "tee",
				type: "SUBSCRIPT",
				board: "XJTUnews",
				timestamp: 1606524800,
			}],
		};
	},
	components: {
		FriendsCard,
	},
}
</script>

<style scoped>
.time, .username { display: inline-block; }
.time { width: 100px; color: #777; }
.username { width: 40px }
</style>

