#ifndef __AOT_PROT_CODE_RANGE_H__
#define __AOT_PROT_CODE_RANGE_H__

namespace aot{ namespace prot{

enum eprot_code_range
{
	e_prot_code_sys_range_begin = 0,
	/// 服务端内部使用
	e_prot_code_sys_range_end = 5000,

	e_prot_code_user_range_begin = 5001,
	/// 服务端内部使用
	e_prot_code_user_range_end = 10000,

	e_prot_code_client_userdef_range_old_begin = 22000,
	/// 警告: 因为历史原因, 这个区段的业务代码因为跟终端的WIN 消息绑定了, 所以这个区段不能再添加任何新的值
	e_prot_code_client_userdef_range_old_end = 22100,

	e_prot_code_tata_range_begin = 30001,
	/// 服务端,客户端公用
	e_prot_code_tata_range_end = 35000,

	e_prot_code_client_userdef_range_new_begin = 40001,
	/// 这个区段用作客户端新的自定义业务代码(t2t, tbm, ebm的自定义业务)
	e_prot_code_client_userdef_range_new_end = 41000,
};

enum ex_code
{
	e_ex_code_tata2tata_direct_transit = 1,
	e_ex_code_ent_broadcast_msg,
	e_ex_code_tribe_broadcast_msg,
	e_ex_code_ent_cc_msg,							/// 企业部门的抄送信息
	e_ex_code_tribe_cc_msg,							/// 群组的抄送信息
	e_ex_code_multicast_msg,						/// 系统多人消息
	e_ex_code_broadcast_msg,						/// 广播消息
	e_ex_code_tata_to_feature_server,
	e_ex_code_d2d_direct_transit,					/// 我的设备 -> 我的设备
	e_ex_code_ims_to_lgs,
};

enum eprot_code
{
	__e_prot_code_sys_begin__ = e_prot_code_sys_range_begin,
	e_prot_code_mps2mps_apns_feedback_token,			/// mps--->mps: mps内部消息,从apns收到的无效token通知
	__e_prot_code_sys_end__ = e_prot_code_sys_range_end,

	__e_prot_code_user_begin__ = e_prot_code_user_range_begin,
	e_prot_code_ims2dbs_regist,
	e_prot_code_ims2dbs_get_ss_list,				/// ims--->dbs: 获取ss地址列表

	e_prot_code_ims2ims_regist,

	e_prot_code_ims2ims_notify_status_change,		/// ims--->ims状态改变

	e_prot_code_get_cluster_info,					/// ss--->集群dbs: 获取集群信息
	e_prot_code_notify_ss_ready,					/// ss--->集群dbs: ss已经准备好,各主机可以连到ss了
	e_prot_code_get_ss_info,						/// sc--->ss: 获取ss信息

	e_prot_code_subscribe_msg_tribe,				/// ims--->tdbs: 向群组服务器订阅群组信息
	e_prot_code_subscribe_msg_ent,					/// ims--->tdbs: 向企业服务器订阅企业信息
	e_prot_code_undo_subscribe_msg_tribe,			/// ims--->tdbs: 向群组服务器取消订阅群组信息
	e_prot_code_undo_subscribe_msg_ent,				/// ims--->tdbs: 向企业服务器取消订阅企业信息

	e_prot_code_regist_to_mques,					/// dbc--->mques: 向数据库服务器注册
	e_prot_code_regist_to_nms,						/// nmc--->nms: 向nms注册
	e_prot_code_im_regist_to_lgs,					/// im lgc--->lgs: 向lgs注册
	e_prot_code_im_regist_to_sms,					/// im smc--->sms: 向save msg server注册
	e_prot_code_im_regist_to_fs,					/// im feature client--->feature server: 向feature server注册

	e_prot_code_ims2dbs_report_addrinfo,			/// im ---->dbs: 发送公网地址
	e_prot_code_dbs2nms_report_load_balance,		/// dbs --->nms: 发送ims负载信息
	e_prot_code_nms2lgs_report_load_balance,		/// nms --->lgs: 发送dbs集群负载信息

	e_prot_code_regist_to_mps,						/// mps client(dbs, ims)--->mps: 向mps注册

	/// msg push service
	e_prot_code_ims2mps_subscribe,					/// ims ---> mps 订阅 消息推送
	e_prot_code_ims2mps_undo_subscribe,				/// ims ---> mps 取消订阅 消息推送
	e_prot_code_ims2mps_notify_tata_status_change,	/// ims ---> mps 取消订阅 tata状态改变

	__e_prot_code_user_end__ = e_prot_code_user_range_end,

	__e_prot_code_tata_begin__ = e_prot_code_tata_range_begin,
	e_prot_code_tata_regist,						/// tata--->ims: 注册		--- unused
	e_prot_code_tata_status_change,					/// tata--->ims: 状态改变
	e_prot_code_tata_chat_msg,						/// tata--->ims: 发送单人聊天消息
	e_prot_code_tribe_chat_msg,						/// tata--->ims: 发送群组聊天消息
	e_prot_code_ent_chat_msg,						/// tata--->ims: 发送企业聊天消息
	e_prot_code_multi_chat_msg,						/// tata--->ims: 发送多人聊天消息
	e_prot_code_d2d_chat_msg,						/// my device--->my device: 发送聊天消息

	e_prot_code_tata_get_tribe_and_ent_info,		/// 该业务代码不再使用-- tata(ims)--->dbs获取用户所参加的群组和所属企业信息
	e_prot_code_tata_get_ent_online_member_list,	/// tata(ims)--->dbs获取用户所属企业的在线成员列表
	e_prot_code_tata_get_tribe_online_member_list,	/// tata(ims)--->dbs获取用户所属群组的在线成员列表

	e_prot_code_get_the_user_status,				/// tata--->ims: 取指定用户的状态信息
	e_prot_code_send_buddy_list,					/// tata--->ims(dbs): 发送好友列表
	e_prot_code_tata2ims_subscribe_msg_tribe,		/// tata--->ims: 订阅群组消息
	e_prot_code_tata2ims_subscribe_msg_ent,			/// tata--->ims: 订阅企业信息
	e_prot_code_tata2ims_undo_subscribe_msg_tribe,	/// tata--->ims: 取消订阅群组消息
	e_prot_code_tata2ims_undo_subscribe_msg_ent,	/// tata--->ims: 取消订阅企业信息

	e_prot_code_del_buddy,							/// 删除好友(
	e_prot_code_add_buddy_invite,					/// 添加好友的请求(未使用)
	e_prot_code_notify_add_buddy_ret,				/// 通知添加好友的结果(未使用)
	e_prot_code_exchange_buddy_info,				/// 交换tata双方信息(添加好友到列表,通过该业务实现)

	e_prot_code_t2t_user_define_msg,				/// tata 与 tata之间交互自定义数据
	e_prot_code_tbm_user_define_msg,				/// 群组广播自定义消息
	e_prot_code_ebm_user_define_msg,				/// 企业部门广播自定义消息
	e_prot_code_d2d_user_define_msg,				/// my device--->my device 自定义消息

	__noused_e_prot_code_read_buddy_offline_msg,				/// dbc--->mques: 读单人的离线消息
	__noused_e_prot_code_write_buddy_offline_msg,				/// dbc--->mques: 写单人的离线消息
	__noused_e_prot_code_sent_buddy_offline_msg,				/// dbc--->mques: 服务端返回单人的离线消息
	__noused_e_prot_code_read_ent_offline_msg,					/// dbc--->mques: 写企业的离线消息
	__noused_e_prot_code_write_ent_offline_msg,					/// dbc--->mques: 取企业的离线消息
	__noused_e_prot_code_sent_ent_offline_msg,					/// dbc--->mques: 服务端返回企业的离线消息
	__noused_e_prot_code_read_tribe_offline_msg,				/// dbc--->mques: 读群组的离线消息
	__noused_e_prot_code_write_tribe_offline_msg,				/// dbc--->mques: 写群组的离线消息
	__noused_e_prot_code_sent_tribe_offline_msg,				/// dbc--->mques: 服务端返回群组的离线消息

	/// 聊天室
	e_prot_code_tata2ims_subscribe_msg_chatroom,	/// tata--->ims: 订阅聊天室信息
	e_prot_code_tata2ims_undo_subscribe_msg_chatroom,/// tata--->ims: 取消订阅聊天室信息
	e_prot_code_chatroom_chat_msg,					/// tata--->ims: 发送聊天室消息
	e_prot_code_cbm_user_define_msg,				/// 聊天室广播自定义消息
	e_prot_code_notify_enter_chat_room,
	e_prot_code_notify_leave_chat_room,
	e_prot_code_notify_member_info,

	///
	e_prot_code_notify_my_device_status_change,		/// tata-->tata(同账号的其他类型终端): 状态改变通知
	e_prot_code_get_userlist_status,				/// 获取指定用户列表的状态
	e_prot_code_get_my_device_list,					/// 获取我的在线设备列表
	e_prot_code_set_tata_runtime_mode,				/// 设置客户端运行时模式
	e_prot_code_notify_user_status_list_change,		/// ims ---> tata 用户状态列表
	
	/// msg push service
	___e_prot_code_MPS_begin__,
	e_prot_code_tata2ims_subscribe_mps,					/// tata ---> ims 订阅 消息推送
	e_prot_code_tata2ims_undo_subscribe_mps,			/// tata ---> ims 取消订阅 消息推送
	___e_prot_code_MPS_end__ = ___e_prot_code_MPS_begin__ + 20,

	___e_prot_code_MQUES_begin__,
	/// 好友
	e_prot_code_MQUES_get_recent_contacts_buddylist,			/// tata --->mques: 获取最近联系人列表

	e_prot_code_MQUES_read_buddy_offline_msg,			/// tata--->mques: 读单人的离线消息
	e_prot_code_MQUES_send_buddy_offline_msg_piece,		/// mques--->tata: 返回读取的单人离线消息的分片包(未使用)
	e_prot_code_MQUES_send_buddy_offline_msg,			/// mques--->tata: MQUES 返回读取的单人离线消息的 完成包
	e_prot_code_MQUES_delete_buddy_offline_msg,			/// tata--->mques: 从MQUES 删除离线消息

	e_prot_code_MQUES_read_buddy_sync_msg,				/// 从MQUES 读取同步消息
	e_prot_code_MQUES_send_buddy_sync_msg,				/// MQUES 返回同步消息
	e_prot_code_MQUES_read_buddylist_last_sync_msg,		/// 从MQUES 读取指定联系人列表的最后一条同步消息
	e_prot_code_MQUES_send_buddylist_last_sync_msg,		/// MQUES 返回指定联系人列表的最后一条同步消息

	/// 企业
	e_prot_code_MQUES_read_ent_dep_sync_msg,						/// tata--->mques: 读企业部门的同步消息
	e_prot_code_MQUES_send_ent_dep_sync_msg,						/// mques--->tata: 返回读取的企业部门同步消息
	e_prot_code_MQUES_read_ent_dep_list_last_sync_msg,			/// tata--->mques: 读企业部门的同步消息
	e_prot_code_MQUES_send_ent_dep_list_last_sync_msg,			/// mques--->tata: 返回读取的企业部门同步消息
	e_prot_code_MQUES_read_ent_dep_list_offline_msg,			/// tata--->mques: 读企业部门的离线消息
	e_prot_code_MQUES_send_ent_dep_list_offline_msg_piece,		/// MQUES 返回读取的企业部门离线消息的 分片包
	e_prot_code_MQUES_send_ent_dep_list_offline_msg,			/// MQUES 返回读取的企业部门离线消息的 完成包

	/// 群组
	e_prot_code_MQUES_read_tribe_sync_msg,						/// tata--->mques: 读群组的同步消息
	e_prot_code_MQUES_send_tribe_sync_msg,						/// mques--->tata: 返回读取的群组同步消息
	e_prot_code_MQUES_read_tribe_list_last_sync_msg,			/// tata--->mques: 读群组的同步消息
	e_prot_code_MQUES_send_tribe_list_last_sync_msg,			/// mques--->tata: 返回读取的群组同步消息
	e_prot_code_MQUES_read_tribe_list_offline_msg,				/// tata--->mques: 读群组的离线消息
	e_prot_code_MQUES_send_tribe_list_offline_msg_piece,		/// MQUES 返回读取的群组离线消息的 分片包
	e_prot_code_MQUES_send_tribe_list_offline_msg,				/// MQUES 返回读取的群组离线消息的 完成包
	

	___e_prot_code_MQUES_end__ = ___e_prot_code_MQUES_begin__ + 50,

	___placeholder0___ = __e_prot_code_tata_begin__ + 450,
	/// 系统消息
	e_prot_code_sys_tbm_msg,						/// 系统群组消息
	e_prot_code_sys_ebm_msg,						/// 系统企业消息
	e_prot_code_sys_t2t_msg,						/// 系统单人消息(暂不使用)
	e_prot_code_sys_multicast_msg,					/// 系统多人消息
	e_prot_code_sys_broadcast_msg,					/// 系统广播消息
	e_prot_code_sys_broadcast_msg_unknow,			/// 系统广播消息

	___placeholder1___ = __e_prot_code_tata_begin__ + 500,		
	e_prot_code_get_online_buddylist,				/// tata--->ims: 在线好友列表
	e_prot_code_notify_status_change,				/// ims--->tata: 好友状态改变
	e_prot_code_notify_tata_forceout,				/// ims--->tata: 强制下线
	e_prot_code_confirm_receipt_ack,				/// ims--->tata: 服务端对客户端"签收消息"的确认(注意: 这里的签收消息 是指: 客户端发过来的消息, 服务端需要确认已经收到)
	
	___placeholder2___ = __e_prot_code_tata_begin__ + 600,	
	/// 登陆
	e_prot_code_lgs_get_login_config_info,			/// 取地址配置信息
	e_prot_code_lgs_get_tata,						/// 取tata号码
	e_prot_code_lgs_tata_auth,						/// 登录验证(密码验证,取token)
	e_prot_code_lgs_get_user_data,					/// 取个人资料
	e_prot_code_lgs_get_tribe_member_list,			/// 取群组成员列表
	e_prot_code_lgs_get_buddy_list,					/// 取好友列表
	e_prot_code_get_bind_info,						/// 取手机号码等绑定信息

	e_prot_code_feature_change_tata_info,			/// 修改个人资料
	e_prot_code_feature_get_buddy_info,				/// 取好友信息
	e_prot_code_feature_get_call_divert,			/// 取呼叫转移配置
	e_prot_code_feature_set_call_divert,			/// 设置呼叫转移配置
	e_prot_code_feature_get_account_balance,		/// 获取号码的帐户余额
	e_prot_code_feature_change_buddy_memo,			/// 修改好友备注
	e_prot_code_feature_get_user_info,				/// 取指定用户的信息
	e_prot_code_feature_get_srv_notify_msg,			/// 读取系统通知消息
	e_prot_code_feature_get_tribe_info,				/// 取群组信息
	e_prot_code_feature_change_tribe_info,			/// 修改群组信息
	e_prot_code_feature_add_buddy_group,			/// 添加好友分组
	e_prot_code_feature_change_buddy_group,			/// 修改好友分组的组名
	e_prot_code_feature_delete_buddy_group,			/// 删除好友分组
	e_prot_code_feature_move_buddy_to_group,		/// 将好友移动到另一个分组
	e_prot_code_feature_add_buddy_to_db,			/// 添加好友(保存到数据库)
	e_prot_code_feature_delete_buddy_to_db,			/// 删除好友(保存到数据库)
	e_prot_code_feature_create_temp_tribe,			/// 创建临时讨论组
	e_prot_code_feature_change_temp_tribe_name,		/// 修改临时讨论组名称
	e_prot_code_feature_add_member_to_temp_tribe,	/// 添加成员到临时讨论组
	e_prot_code_feature_exit_temp_tribe,			/// 退出临时讨论组
	e_prot_code_feature_add_member_to_tribe,		/// 添加成员到群组
	e_prot_code_feature_delete_tribe,				/// 删除群组
	e_prot_code_feature_set_tribe_member_power,		/// 设置群组成员角色
	e_prot_code_feature_tribe_transfer_power,		/// 群组成员身份转移
	e_prot_code_feature_delete_tribe_member,		/// 删除群组成员
	e_prot_code_feature_get_tribe_member_list,		/// 获取群组成员列表
	e_prot_code_feature_get_buddy_list,				/// 取好友列表
	e_prot_code_feature_get_user_videomeeting_status,	/// 取指定用户视频会议状态(是否在会议中)

	e_prot_code_feature_tata_login_regist,			/// tata 登录ims时, 通过 feature server 到数据库注册
	e_prot_code_feature_tata_keeplive_regist,		/// tata 通过 feature server 定时到数据库注册
	e_prot_code_feature_tata_regist_out,			/// tata 与ims断链时, ims 通过 feature server 到数据库反注册
													/// 注意: 这个业务是由ims发起的

	e_prot_code_feature_create_tribe,				/// 创建群组
	
	___placeholder3___ = __e_prot_code_tata_begin__ +700,

	e_prot_code_get_ent_videomeeting_info,			/// 获取视频会议属性,登录时获取

	
	__e_prot_code_tata_end__ = e_prot_code_tata_range_end,

	
};

}} /// end namespace aot/prot

#endif /// __AOT_PROT_CODE_RANGE_H__