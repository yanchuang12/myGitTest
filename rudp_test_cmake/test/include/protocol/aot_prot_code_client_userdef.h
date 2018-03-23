#ifndef __AOT_PROT_CODE_CLEINT_USERDEF_H__
#define __AOT_PROT_CODE_CLEINT_USERDEF_H__

#include "aot_prot_code_range_def.h"

namespace aot{ namespace prot{

enum eclient_userdef_prot_code_old
{
	/// 警告: 因为历史原因, 这些旧的业务代码因为跟终端的WIN 消息绑定了, 所以这里不能再添加任何新的值
	/// 注意: 所有在这里新增的自定义消息, 都需要在 src\component\aot_im\udmsg_to_wmmsg.cpp 文件内增加一条对应的与WIN消息对应的记录
	__eclient_userdef_code_old_begin__ = aot::prot::e_prot_code_client_userdef_range_old_begin,	/// 22000

	e_client_userdef_prot_code_add_buddy_invite = 22034,				/// t2t WM_GLB_IM2UI_ADD_BUDDY_INVITE
	e_client_userdef_prot_code_add_buddy_ret = 22035,					/// t2t WM_GLB_IM2UI_ADD_BUDDY_RET
	e_client_userdef_prot_code_can_you_join_to_tribe = 22036,			/// t2t WM_GLB_IM2UI_CAN_YOU_JOIN_TO_TRIBE
	e_client_userdef_prot_code_can_i_join_to_tribe = 22037,				/// t2t WM_GLB_IM2UI_CAN_I_JOIN_TO_TRIBE
	e_client_userdef_prot_code_join_to_tribe_ret = 22038,				/// t2t WM_GLB_IM2UI_JOIN_TO_TRIBE_RET
	e_client_userdef_prot_code_tribe_add_new_member = 22039,			/// tbm WM_GLB_IM2UI_NOTIFY_TRIBE_ADD_NEW_MEMBER
	e_client_userdef_prot_code_notify_tribe_del_member = 22040,			/// tbm WM_GLB_IM2UI_NOTIFY_TRIBE_DEL_MEMBER
	e_client_userdef_prot_code_notify_exit_tribe = 22041,				/// tbm WM_GLB_IM2UI_NOTIFY_EXIT_TRIBE
	e_client_userdef_prot_code_notify_dissolve_tribe = 22042,			/// tbm WM_GLB_IM2UI_NOTIFY_DISSOLVE_TRIBE
	e_client_userdef_prot_code_notify_tribe_update_notice_msg = 22043,	/// tbm WM_GLB_IM2UI_NOTOFY_TRIBE_UPDATE_NOTICE_MSG
	e_client_userdef_prot_code_notify_tribe_change_member_power = 22044,	/// tbm WM_GLB_IM2UI_NOTIFY_TRIBE_CHANGE_MEMBER_POWER
	e_client_userdef_prot_code_notify_tribe_transfer_power = 22045,		/// tbm WM_GLB_IM2UI_NOTIFY_TRIBE_TRANSFER_POWER
	e_client_userdef_prot_code_video_meeting_invite = 22046,			/// t2t WM_GLB_IM2UI_VIDEO_MEETING_INVITE
	e_client_userdef_prot_code_video_meeting_ret = 22047,				/// t2t WM_GLB_IM2UI_VIDEO_MEETING_RET
	e_client_userdef_prot_code_chatroom_notify_member_info = 22048,		/// cbm(未实现) WM_GLB_IM2UI_CHATROOM_NOTIFY_MEMBER_INFO

	e_client_userdef_prot_code_exchange_user_person_data = 22057,		/// t2t WM_GLB_IM2UI_EXCHANGE_USER_PERSON_DATA

	e_client_userdef_prot_code_tribe_send_file = 22063,					/// tbm WM_GLB_IM2UI_TRIBE_SEND_FILE
	e_client_userdef_prot_code_ent_send_file = 22064,					/// ebm WM_GLB_IM2UI_ENT_SEND_FILE

	/// 还有一些旧的自定义命令,未整理进来的, 在这里继续...

	__eclient_userdef_code_old_end__ = aot::prot::e_prot_code_client_userdef_range_old_end,/// 22100
};

enum eclient_userdef_prot_code_new
{
	/// 注意: 所有在这里新增的自定义消息, 都需要在 src\component\aot_im\udmsg_to_wmmsg.cpp 文件内增加一条对应的与WIN消息对应的记录
	__eclient_userdef_code_new_begin__ = aot::prot::e_prot_code_client_userdef_range_new_begin,	/// 40001

	/* tata -->tata 自定义消息区间 */
	__eclient_userdef_code_new_t2t_begin__,
	e_client_userdef_code_new_t2t_chat_inputting,									/// 对方正在输入 @pkt->session_info.sender_inner_id: 对方inner_id

	/// 在线文件传输自定义消息
	__eclient_userdef_code_new_t2t_online_file_trans_begin__,
	e_client_userdef_code_new_t2t_online_file_trans_sender_request,						/// 文件发送请求
	e_client_userdef_code_new_t2t_online_file_trans_sender_cancel,						/// 发送方取消发送/中途取消
	e_client_userdef_code_new_t2t_online_file_trans_sender_change_to_offline,			/// 发送方该为离线传输
	e_client_userdef_code_new_t2t_online_file_trans_recver_accept,						/// 接收方接收
	e_client_userdef_code_new_t2t_online_file_trans_recver_reject,						/// 接收方拒绝
	e_client_userdef_code_new_t2t_online_file_trans_recver_cancel,						/// 接收方中途取消
	e_client_userdef_code_new_t2t_online_file_trans_recver_high_security_level,			/// 接收方文件传输安全级设置太高
	__eclient_userdef_code_new_t2t_online_file_trans_end__ = __eclient_userdef_code_new_t2t_online_file_trans_begin__ + 50,

	/// 远程协助自定义消息
	__eclient_userdef_code_new_t2t_rd_begin__,
	e_client_userdef_code_new_t2t_rd_invite,						/// rds -> rdc: 申请远程协助请求
	e_client_userdef_code_new_t2t_rd_invite_ack,					/// rdc -> rds: 接受/拒绝 远程协助
	e_client_userdef_code_new_t2t_rd_addr_info,						/// rdc -> rds: 发送自己的地址给对方
	e_client_userdef_code_new_t2t_rd_addr_info_ack,					/// rds -> rdc: 发送自己的地址给对方
	e_client_userdef_code_new_t2t_rd_meeting,						/// rdc -> rds: 远程协助已建立连接,并可以查看对方屏幕
	e_client_userdef_code_new_t2t_rd_enable_input_invite,			/// rds -> rdc: 申请控制
	e_client_userdef_code_new_t2t_rd_enable_input_invite_ack,		/// rdc -> rds: 接受/拒绝 控制
	e_client_userdef_code_new_t2t_rd_disable_input_invite,			/// 双方都可:   释放控制请求
	e_client_userdef_code_new_t2t_rd_disable_input_invite_ack,		/// 双方都可:   释放控制应答
	e_client_userdef_code_new_t2t_rd_reset,							/// 双方都可:   重置远程协助(界面无需提示)
	e_client_userdef_code_new_t2t_rd_abort,							/// 双方都可:	断开远程协助
	__eclient_userdef_code_new_t2t_rd_end__ = __eclient_userdef_code_new_t2t_rd_begin__ + 50,

	/// 窗口抖动自定义消息
	e_client_userdef_code_new_t2t_window_shake,
	/// 自动回复自定义消息
	e_client_userdef_code_new_t2t_auto_reply,
	/// 单人离线文件传输
	e_client_userdef_code_new_t2t_offline_file_trans,

	__eclient_userdef_code_new_t2t_end__ = __eclient_userdef_code_new_t2t_begin__ + 200,

	/// ebm 自定义消息区间
	__eclient_userdef_code_new_ebm_begin__,
	__eclient_userdef_code_new_ebm_end__ = __eclient_userdef_code_new_ebm_begin__ + 100,

	/// tbm 自定义消息区间
	__eclient_userdef_code_new_tbm_begin__,
	__eclient_userdef_code_new_tbm_end__ = __eclient_userdef_code_new_tbm_begin__ + 100,

	/* 我的设备 -> 我的设备 自定义消息区间 */
	__eclient_userdef_code_new_d2d_begin__,
	e_client_userdef_code_new_d2d_msg_viewed,							/// 通知我的其他设备某类型消息已经查看过了
	e_client_userdef_code_new_d2d_add_buddy_ret,						/// 通知我的其他设备已经同意/拒绝对方好友邀请了
	e_client_userdef_code_new_d2d_delete_buddy,							/// 通知我的其他设备已经删除好友了
	e_client_userdef_code_new_d2d_join_tribe_ret,						/// 通知我的其他设备已经同意/拒绝加入群组了
	e_client_userdef_code_new_d2d_add_member_to_temp_tribe,				/// 通知我的其他设备添加了讨论组成员
	e_client_userdef_code_new_d2d_create_tribe_ret,						/// 通知我的其他设备创建了群组
	e_client_userdef_code_new_d2d_delete_tribe_member,					/// 通知我的其他设备删除了某个成员
	e_client_userdef_code_new_d2d_quit_tribe,							/// 通知我的其他设备退出了某个群
	e_client_userdef_code_new_d2d_delete_tribe,							/// 通知我的其他设备解散了某个群
	e_client_userdef_code_new_d2d_quit_temp_tribe,						/// 通知我的其他设备退出了某个讨论组
	e_client_userdef_code_new_d2d_change_tribe_member_pow,				/// 通知我的其他设备指定/撤销了某个成员副管理员
	e_client_userdef_code_new_d2d_create_temp_tribe_ret,				/// 通知我的其他设备创建了讨论组
	e_client_userdef_code_new_d2d_add_buddy_group_ret,					/// 通知我的其他设备添加了好友分组
	e_client_userdef_code_new_d2d_change_buddy_group,					/// 通知我的其他设备修改了好友分组名
	e_client_userdef_code_new_d2d_delete_buddy_group,					/// 通知我的其他设备删除了好友分组
	e_client_userdef_code_new_d2d_move_buddy_to_group,					/// 通知我的其他设备移动了好友到其他分组
	__eclient_userdef_code_new_d2d_end__ = __eclient_userdef_code_new_d2d_begin__ + 200,

	/// 特定普通聊天消息区间
	__eclient_userdef_code_new_chat_msg_begine__,
	/// 语音对讲
	eclient_userdef_code_new_chat_msg_inerphone_send_sound,
	/// 地理位置
	eclient_userdef_code_new_chat_msg_location_map,
	__e_client_userdef_code_new_chat_msg_end__ = __eclient_userdef_code_new_chat_msg_begine__ + 100,

	/// end pos
	__eclient_userdef_code_new_end__ = aot::prot::e_prot_code_client_userdef_range_new_end,		/// 41000
};

}} /// end namespace aot/prot

#endif /// __AOT_PROT_CODE_CLEINT_USERDEF_H__