import { createApp } from "vue";
import App from "./App.vue";
import router from "./router";
import 'bootstrap'
import 'bootstrap/dist/css/bootstrap.min.css'
import { FontAwesomeIcon } from "./plugins/font-awesome.js"
import "@/assets/aha.scss"

createApp(App)
	.use(router)
	.component('fa', FontAwesomeIcon)
	.mount("#app");
