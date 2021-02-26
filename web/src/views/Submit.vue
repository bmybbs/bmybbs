<template>
	<div>
		<h5>您有什么想和大家分享的...</h5>

		<div class="bmy-post-dropdown">
			<div class="input-group input-group-lg flex-nowrap bmy-post">
				<input id="bmy-post-board" type="text" class="form-control bmy-post-choose-board" @focus="searchFocus" @blur="searchBlur" @keyup="doLasySearch" placeholder="请选择或搜索版面" v-model="board">
				<span class="input-group-text" id="addon-wrapping">
					<fa icon="search" v-if="isSearching" />
					<fa icon="spinner" v-else />
				</span>
			</div>

			<div class="bmy-post-board-list shadow rounded" :class="{ show: isSearching }" v-if="filtered.length > 0">
				<div v-for="board in filtered" :key="board.name" class="px-3 py-2 bmy-post-board-list-item" @click="pickBoard(board.name)" @mouseenter="isPickingBoard = true" @mouseout="isPickingBoard = false">
					{{ board.name }}
				</div>
			</div>
			<div v-else class="bmy-post-board-list shadow rounded" :class="{ show: isSearching }">
				<div class="px-3 py-2 bmy-post-board-list-item text-danger">没有对应版面</div>
			</div>
		</div>

		<TabbedEditor :_boardname_en="board"/>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import TabbedEditor from "@/components/TabbedEditor.vue"

export default {
	data() {
		return {
			board: this.$route.name == "RAWSUBMIT" ? "" : this.$route.params.boardname,
			isSearching: false,
			isPickingBoard: false,
			favboards: [],
			lazySearchTimer: null,
			filtered: []
		};
	},
	created() {
		this.$watch(() => this.$route, (toRoute) => {
			this.board = toRoute.name == "RAWSUBMIT" ? "" : toRoute.params.boardname;
		});
	},
	mounted() {
	},
	methods: {
		searchFocus() {
			this.isSearching = true;
			this.load_favboards();
		},
		searchBlur() {
			if (!this.isPickingBoard) {
				this.isSearching = false;
			}
		},
		load_favboards() {
			BMYClient.get_fav_board_list().then(response => {
				if (response.errcode == 0 && Array.isArray(response.board_array)) {
					this.favboards = response.board_array;
					this.filtered = this.favboards;
				}
			});
		},
		pickBoard(board) {
			this.board = board;
			this.$router.push({
				name: "boardSubmit",
				params: {
					boardname: board,
				}
			});
			this.isPickingBoard = false;
			this.isSearching = false;
		},
		doLasySearch() {
			if (this.lazySearchTimer) {
				clearTimeout(this.lazySearchTimer);
			}

			this.lazySearchTimer = setTimeout(() => {
				if (this.board == "") {
					this.filtered = this.favboards;
				} else {
					BMYClient.search_board(this.board).then(response => {
						if (response.errcode == 0 && Array.isArray(response.board_array)) {
							this.filtered = response.board_array;
						} else {
							this.filtered = [];
						}
					});
				}
			}, 200);
		},
	},
	components: {
		TabbedEditor,
	},
}
</script>

<style scoped>
.bmy-post-dropdown {
	position: relative;
}

.bmy-post .input-group-text {
	background: #fff;
	color: #555;
}

.bmy-post .bmy-post-choose-board {
	border-right: 0px;
}

.bmy-post .bmy-post-choose-board:focus {
	border-color: rgb(206, 212, 218);
	box-shadow: initial;
}

.bmy-post-dropdown .bmy-post-board-list {
	display: none;
	position: absolute;
	z-index: 5000;
	background-color: #fff;
	width: 100%;
	max-height: 300px;
	overflow-x: hidden;
	overflow-y: scroll;
}

.bmy-post-dropdown .bmy-post-board-list.show {
	display: block;
}

.bmy-post-board-list-item:hover {
	background: #efefef;
	cursor: pointer;
}
</style>

