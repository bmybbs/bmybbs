import dayjs from "dayjs";
import relativeTime from "dayjs/plugin/relativeTime"
import localizedFormat from "dayjs/plugin/localizedFormat"
import "dayjs/locale/zh-cn"

dayjs.locale("zh-cn");
dayjs.extend(relativeTime);
dayjs.extend(localizedFormat);

export const getReadableTime = function(unix_timestamp) {
	return dayjs(unix_timestamp * 1000).fromNow();
};

export const getLongVersionTime = function(unix_timestamp) {
	return dayjs(unix_timestamp * 1000).format("lll");
}

