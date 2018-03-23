#ifndef __AOT_PROT_CODE_RANGE_H__
#define __AOT_PROT_CODE_RANGE_H__

namespace aot{ namespace prot{

enum eprot_code_range
{
	e_prot_code_sys_range_begin = 0,
	/// ������ڲ�ʹ��
	e_prot_code_sys_range_end = 5000,

	e_prot_code_user_range_begin = 5001,
	/// ������ڲ�ʹ��
	e_prot_code_user_range_end = 10000,

	e_prot_code_client_userdef_range_old_begin = 22000,
	/// ����: ��Ϊ��ʷԭ��, ������ε�ҵ�������Ϊ���ն˵�WIN ��Ϣ����, ����������β���������κ��µ�ֵ
	e_prot_code_client_userdef_range_old_end = 22100,

	e_prot_code_tata_range_begin = 30001,
	/// �����,�ͻ��˹���
	e_prot_code_tata_range_end = 35000,

	e_prot_code_client_userdef_range_new_begin = 40001,
	/// ������������ͻ����µ��Զ���ҵ�����(t2t, tbm, ebm���Զ���ҵ��)
	e_prot_code_client_userdef_range_new_end = 41000,
};

enum ex_code
{
	e_ex_code_tata2tata_direct_transit = 1,
	e_ex_code_ent_broadcast_msg,
	e_ex_code_tribe_broadcast_msg,
	e_ex_code_ent_cc_msg,							/// ��ҵ���ŵĳ�����Ϣ
	e_ex_code_tribe_cc_msg,							/// Ⱥ��ĳ�����Ϣ
	e_ex_code_multicast_msg,						/// ϵͳ������Ϣ
	e_ex_code_broadcast_msg,						/// �㲥��Ϣ
	e_ex_code_tata_to_feature_server,
	e_ex_code_d2d_direct_transit,					/// �ҵ��豸 -> �ҵ��豸
	e_ex_code_ims_to_lgs,
};

enum eprot_code
{
	__e_prot_code_sys_begin__ = e_prot_code_sys_range_begin,
	e_prot_code_mps2mps_apns_feedback_token,			/// mps--->mps: mps�ڲ���Ϣ,��apns�յ�����Чtoken֪ͨ
	__e_prot_code_sys_end__ = e_prot_code_sys_range_end,

	__e_prot_code_user_begin__ = e_prot_code_user_range_begin,
	e_prot_code_ims2dbs_regist,
	e_prot_code_ims2dbs_get_ss_list,				/// ims--->dbs: ��ȡss��ַ�б�

	e_prot_code_ims2ims_regist,

	e_prot_code_ims2ims_notify_status_change,		/// ims--->ims״̬�ı�

	e_prot_code_get_cluster_info,					/// ss--->��Ⱥdbs: ��ȡ��Ⱥ��Ϣ
	e_prot_code_notify_ss_ready,					/// ss--->��Ⱥdbs: ss�Ѿ�׼����,��������������ss��
	e_prot_code_get_ss_info,						/// sc--->ss: ��ȡss��Ϣ

	e_prot_code_subscribe_msg_tribe,				/// ims--->tdbs: ��Ⱥ�����������Ⱥ����Ϣ
	e_prot_code_subscribe_msg_ent,					/// ims--->tdbs: ����ҵ������������ҵ��Ϣ
	e_prot_code_undo_subscribe_msg_tribe,			/// ims--->tdbs: ��Ⱥ�������ȡ������Ⱥ����Ϣ
	e_prot_code_undo_subscribe_msg_ent,				/// ims--->tdbs: ����ҵ������ȡ��������ҵ��Ϣ

	e_prot_code_regist_to_mques,					/// dbc--->mques: �����ݿ������ע��
	e_prot_code_regist_to_nms,						/// nmc--->nms: ��nmsע��
	e_prot_code_im_regist_to_lgs,					/// im lgc--->lgs: ��lgsע��
	e_prot_code_im_regist_to_sms,					/// im smc--->sms: ��save msg serverע��
	e_prot_code_im_regist_to_fs,					/// im feature client--->feature server: ��feature serverע��

	e_prot_code_ims2dbs_report_addrinfo,			/// im ---->dbs: ���͹�����ַ
	e_prot_code_dbs2nms_report_load_balance,		/// dbs --->nms: ����ims������Ϣ
	e_prot_code_nms2lgs_report_load_balance,		/// nms --->lgs: ����dbs��Ⱥ������Ϣ

	e_prot_code_regist_to_mps,						/// mps client(dbs, ims)--->mps: ��mpsע��

	/// msg push service
	e_prot_code_ims2mps_subscribe,					/// ims ---> mps ���� ��Ϣ����
	e_prot_code_ims2mps_undo_subscribe,				/// ims ---> mps ȡ������ ��Ϣ����
	e_prot_code_ims2mps_notify_tata_status_change,	/// ims ---> mps ȡ������ tata״̬�ı�

	__e_prot_code_user_end__ = e_prot_code_user_range_end,

	__e_prot_code_tata_begin__ = e_prot_code_tata_range_begin,
	e_prot_code_tata_regist,						/// tata--->ims: ע��		--- unused
	e_prot_code_tata_status_change,					/// tata--->ims: ״̬�ı�
	e_prot_code_tata_chat_msg,						/// tata--->ims: ���͵���������Ϣ
	e_prot_code_tribe_chat_msg,						/// tata--->ims: ����Ⱥ��������Ϣ
	e_prot_code_ent_chat_msg,						/// tata--->ims: ������ҵ������Ϣ
	e_prot_code_multi_chat_msg,						/// tata--->ims: ���Ͷ���������Ϣ
	e_prot_code_d2d_chat_msg,						/// my device--->my device: ����������Ϣ

	e_prot_code_tata_get_tribe_and_ent_info,		/// ��ҵ����벻��ʹ��-- tata(ims)--->dbs��ȡ�û����μӵ�Ⱥ���������ҵ��Ϣ
	e_prot_code_tata_get_ent_online_member_list,	/// tata(ims)--->dbs��ȡ�û�������ҵ�����߳�Ա�б�
	e_prot_code_tata_get_tribe_online_member_list,	/// tata(ims)--->dbs��ȡ�û�����Ⱥ������߳�Ա�б�

	e_prot_code_get_the_user_status,				/// tata--->ims: ȡָ���û���״̬��Ϣ
	e_prot_code_send_buddy_list,					/// tata--->ims(dbs): ���ͺ����б�
	e_prot_code_tata2ims_subscribe_msg_tribe,		/// tata--->ims: ����Ⱥ����Ϣ
	e_prot_code_tata2ims_subscribe_msg_ent,			/// tata--->ims: ������ҵ��Ϣ
	e_prot_code_tata2ims_undo_subscribe_msg_tribe,	/// tata--->ims: ȡ������Ⱥ����Ϣ
	e_prot_code_tata2ims_undo_subscribe_msg_ent,	/// tata--->ims: ȡ��������ҵ��Ϣ

	e_prot_code_del_buddy,							/// ɾ������(
	e_prot_code_add_buddy_invite,					/// ��Ӻ��ѵ�����(δʹ��)
	e_prot_code_notify_add_buddy_ret,				/// ֪ͨ��Ӻ��ѵĽ��(δʹ��)
	e_prot_code_exchange_buddy_info,				/// ����tata˫����Ϣ(��Ӻ��ѵ��б�,ͨ����ҵ��ʵ��)

	e_prot_code_t2t_user_define_msg,				/// tata �� tata֮�佻���Զ�������
	e_prot_code_tbm_user_define_msg,				/// Ⱥ��㲥�Զ�����Ϣ
	e_prot_code_ebm_user_define_msg,				/// ��ҵ���Ź㲥�Զ�����Ϣ
	e_prot_code_d2d_user_define_msg,				/// my device--->my device �Զ�����Ϣ

	__noused_e_prot_code_read_buddy_offline_msg,				/// dbc--->mques: �����˵�������Ϣ
	__noused_e_prot_code_write_buddy_offline_msg,				/// dbc--->mques: д���˵�������Ϣ
	__noused_e_prot_code_sent_buddy_offline_msg,				/// dbc--->mques: ����˷��ص��˵�������Ϣ
	__noused_e_prot_code_read_ent_offline_msg,					/// dbc--->mques: д��ҵ��������Ϣ
	__noused_e_prot_code_write_ent_offline_msg,					/// dbc--->mques: ȡ��ҵ��������Ϣ
	__noused_e_prot_code_sent_ent_offline_msg,					/// dbc--->mques: ����˷�����ҵ��������Ϣ
	__noused_e_prot_code_read_tribe_offline_msg,				/// dbc--->mques: ��Ⱥ���������Ϣ
	__noused_e_prot_code_write_tribe_offline_msg,				/// dbc--->mques: дȺ���������Ϣ
	__noused_e_prot_code_sent_tribe_offline_msg,				/// dbc--->mques: ����˷���Ⱥ���������Ϣ

	/// ������
	e_prot_code_tata2ims_subscribe_msg_chatroom,	/// tata--->ims: ������������Ϣ
	e_prot_code_tata2ims_undo_subscribe_msg_chatroom,/// tata--->ims: ȡ��������������Ϣ
	e_prot_code_chatroom_chat_msg,					/// tata--->ims: ������������Ϣ
	e_prot_code_cbm_user_define_msg,				/// �����ҹ㲥�Զ�����Ϣ
	e_prot_code_notify_enter_chat_room,
	e_prot_code_notify_leave_chat_room,
	e_prot_code_notify_member_info,

	///
	e_prot_code_notify_my_device_status_change,		/// tata-->tata(ͬ�˺ŵ����������ն�): ״̬�ı�֪ͨ
	e_prot_code_get_userlist_status,				/// ��ȡָ���û��б��״̬
	e_prot_code_get_my_device_list,					/// ��ȡ�ҵ������豸�б�
	e_prot_code_set_tata_runtime_mode,				/// ���ÿͻ�������ʱģʽ
	e_prot_code_notify_user_status_list_change,		/// ims ---> tata �û�״̬�б�
	
	/// msg push service
	___e_prot_code_MPS_begin__,
	e_prot_code_tata2ims_subscribe_mps,					/// tata ---> ims ���� ��Ϣ����
	e_prot_code_tata2ims_undo_subscribe_mps,			/// tata ---> ims ȡ������ ��Ϣ����
	___e_prot_code_MPS_end__ = ___e_prot_code_MPS_begin__ + 20,

	___e_prot_code_MQUES_begin__,
	/// ����
	e_prot_code_MQUES_get_recent_contacts_buddylist,			/// tata --->mques: ��ȡ�����ϵ���б�

	e_prot_code_MQUES_read_buddy_offline_msg,			/// tata--->mques: �����˵�������Ϣ
	e_prot_code_MQUES_send_buddy_offline_msg_piece,		/// mques--->tata: ���ض�ȡ�ĵ���������Ϣ�ķ�Ƭ��(δʹ��)
	e_prot_code_MQUES_send_buddy_offline_msg,			/// mques--->tata: MQUES ���ض�ȡ�ĵ���������Ϣ�� ��ɰ�
	e_prot_code_MQUES_delete_buddy_offline_msg,			/// tata--->mques: ��MQUES ɾ��������Ϣ

	e_prot_code_MQUES_read_buddy_sync_msg,				/// ��MQUES ��ȡͬ����Ϣ
	e_prot_code_MQUES_send_buddy_sync_msg,				/// MQUES ����ͬ����Ϣ
	e_prot_code_MQUES_read_buddylist_last_sync_msg,		/// ��MQUES ��ȡָ����ϵ���б�����һ��ͬ����Ϣ
	e_prot_code_MQUES_send_buddylist_last_sync_msg,		/// MQUES ����ָ����ϵ���б�����һ��ͬ����Ϣ

	/// ��ҵ
	e_prot_code_MQUES_read_ent_dep_sync_msg,						/// tata--->mques: ����ҵ���ŵ�ͬ����Ϣ
	e_prot_code_MQUES_send_ent_dep_sync_msg,						/// mques--->tata: ���ض�ȡ����ҵ����ͬ����Ϣ
	e_prot_code_MQUES_read_ent_dep_list_last_sync_msg,			/// tata--->mques: ����ҵ���ŵ�ͬ����Ϣ
	e_prot_code_MQUES_send_ent_dep_list_last_sync_msg,			/// mques--->tata: ���ض�ȡ����ҵ����ͬ����Ϣ
	e_prot_code_MQUES_read_ent_dep_list_offline_msg,			/// tata--->mques: ����ҵ���ŵ�������Ϣ
	e_prot_code_MQUES_send_ent_dep_list_offline_msg_piece,		/// MQUES ���ض�ȡ����ҵ����������Ϣ�� ��Ƭ��
	e_prot_code_MQUES_send_ent_dep_list_offline_msg,			/// MQUES ���ض�ȡ����ҵ����������Ϣ�� ��ɰ�

	/// Ⱥ��
	e_prot_code_MQUES_read_tribe_sync_msg,						/// tata--->mques: ��Ⱥ���ͬ����Ϣ
	e_prot_code_MQUES_send_tribe_sync_msg,						/// mques--->tata: ���ض�ȡ��Ⱥ��ͬ����Ϣ
	e_prot_code_MQUES_read_tribe_list_last_sync_msg,			/// tata--->mques: ��Ⱥ���ͬ����Ϣ
	e_prot_code_MQUES_send_tribe_list_last_sync_msg,			/// mques--->tata: ���ض�ȡ��Ⱥ��ͬ����Ϣ
	e_prot_code_MQUES_read_tribe_list_offline_msg,				/// tata--->mques: ��Ⱥ���������Ϣ
	e_prot_code_MQUES_send_tribe_list_offline_msg_piece,		/// MQUES ���ض�ȡ��Ⱥ��������Ϣ�� ��Ƭ��
	e_prot_code_MQUES_send_tribe_list_offline_msg,				/// MQUES ���ض�ȡ��Ⱥ��������Ϣ�� ��ɰ�
	

	___e_prot_code_MQUES_end__ = ___e_prot_code_MQUES_begin__ + 50,

	___placeholder0___ = __e_prot_code_tata_begin__ + 450,
	/// ϵͳ��Ϣ
	e_prot_code_sys_tbm_msg,						/// ϵͳȺ����Ϣ
	e_prot_code_sys_ebm_msg,						/// ϵͳ��ҵ��Ϣ
	e_prot_code_sys_t2t_msg,						/// ϵͳ������Ϣ(�ݲ�ʹ��)
	e_prot_code_sys_multicast_msg,					/// ϵͳ������Ϣ
	e_prot_code_sys_broadcast_msg,					/// ϵͳ�㲥��Ϣ
	e_prot_code_sys_broadcast_msg_unknow,			/// ϵͳ�㲥��Ϣ

	___placeholder1___ = __e_prot_code_tata_begin__ + 500,		
	e_prot_code_get_online_buddylist,				/// tata--->ims: ���ߺ����б�
	e_prot_code_notify_status_change,				/// ims--->tata: ����״̬�ı�
	e_prot_code_notify_tata_forceout,				/// ims--->tata: ǿ������
	e_prot_code_confirm_receipt_ack,				/// ims--->tata: ����˶Կͻ���"ǩ����Ϣ"��ȷ��(ע��: �����ǩ����Ϣ ��ָ: �ͻ��˷���������Ϣ, �������Ҫȷ���Ѿ��յ�)
	
	___placeholder2___ = __e_prot_code_tata_begin__ + 600,	
	/// ��½
	e_prot_code_lgs_get_login_config_info,			/// ȡ��ַ������Ϣ
	e_prot_code_lgs_get_tata,						/// ȡtata����
	e_prot_code_lgs_tata_auth,						/// ��¼��֤(������֤,ȡtoken)
	e_prot_code_lgs_get_user_data,					/// ȡ��������
	e_prot_code_lgs_get_tribe_member_list,			/// ȡȺ���Ա�б�
	e_prot_code_lgs_get_buddy_list,					/// ȡ�����б�
	e_prot_code_get_bind_info,						/// ȡ�ֻ�����Ȱ���Ϣ

	e_prot_code_feature_change_tata_info,			/// �޸ĸ�������
	e_prot_code_feature_get_buddy_info,				/// ȡ������Ϣ
	e_prot_code_feature_get_call_divert,			/// ȡ����ת������
	e_prot_code_feature_set_call_divert,			/// ���ú���ת������
	e_prot_code_feature_get_account_balance,		/// ��ȡ������ʻ����
	e_prot_code_feature_change_buddy_memo,			/// �޸ĺ��ѱ�ע
	e_prot_code_feature_get_user_info,				/// ȡָ���û�����Ϣ
	e_prot_code_feature_get_srv_notify_msg,			/// ��ȡϵͳ֪ͨ��Ϣ
	e_prot_code_feature_get_tribe_info,				/// ȡȺ����Ϣ
	e_prot_code_feature_change_tribe_info,			/// �޸�Ⱥ����Ϣ
	e_prot_code_feature_add_buddy_group,			/// ��Ӻ��ѷ���
	e_prot_code_feature_change_buddy_group,			/// �޸ĺ��ѷ��������
	e_prot_code_feature_delete_buddy_group,			/// ɾ�����ѷ���
	e_prot_code_feature_move_buddy_to_group,		/// �������ƶ�����һ������
	e_prot_code_feature_add_buddy_to_db,			/// ��Ӻ���(���浽���ݿ�)
	e_prot_code_feature_delete_buddy_to_db,			/// ɾ������(���浽���ݿ�)
	e_prot_code_feature_create_temp_tribe,			/// ������ʱ������
	e_prot_code_feature_change_temp_tribe_name,		/// �޸���ʱ����������
	e_prot_code_feature_add_member_to_temp_tribe,	/// ��ӳ�Ա����ʱ������
	e_prot_code_feature_exit_temp_tribe,			/// �˳���ʱ������
	e_prot_code_feature_add_member_to_tribe,		/// ��ӳ�Ա��Ⱥ��
	e_prot_code_feature_delete_tribe,				/// ɾ��Ⱥ��
	e_prot_code_feature_set_tribe_member_power,		/// ����Ⱥ���Ա��ɫ
	e_prot_code_feature_tribe_transfer_power,		/// Ⱥ���Ա���ת��
	e_prot_code_feature_delete_tribe_member,		/// ɾ��Ⱥ���Ա
	e_prot_code_feature_get_tribe_member_list,		/// ��ȡȺ���Ա�б�
	e_prot_code_feature_get_buddy_list,				/// ȡ�����б�
	e_prot_code_feature_get_user_videomeeting_status,	/// ȡָ���û���Ƶ����״̬(�Ƿ��ڻ�����)

	e_prot_code_feature_tata_login_regist,			/// tata ��¼imsʱ, ͨ�� feature server �����ݿ�ע��
	e_prot_code_feature_tata_keeplive_regist,		/// tata ͨ�� feature server ��ʱ�����ݿ�ע��
	e_prot_code_feature_tata_regist_out,			/// tata ��ims����ʱ, ims ͨ�� feature server �����ݿⷴע��
													/// ע��: ���ҵ������ims�����

	e_prot_code_feature_create_tribe,				/// ����Ⱥ��
	
	___placeholder3___ = __e_prot_code_tata_begin__ +700,

	e_prot_code_get_ent_videomeeting_info,			/// ��ȡ��Ƶ��������,��¼ʱ��ȡ

	
	__e_prot_code_tata_end__ = e_prot_code_tata_range_end,

	
};

}} /// end namespace aot/prot

#endif /// __AOT_PROT_CODE_RANGE_H__