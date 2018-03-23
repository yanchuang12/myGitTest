#ifndef __AOT_PROT_CODE_CLEINT_USERDEF_H__
#define __AOT_PROT_CODE_CLEINT_USERDEF_H__

#include "aot_prot_code_range_def.h"

namespace aot{ namespace prot{

enum eclient_userdef_prot_code_old
{
	/// ����: ��Ϊ��ʷԭ��, ��Щ�ɵ�ҵ�������Ϊ���ն˵�WIN ��Ϣ����, �������ﲻ��������κ��µ�ֵ
	/// ע��: �����������������Զ�����Ϣ, ����Ҫ�� src\component\aot_im\udmsg_to_wmmsg.cpp �ļ�������һ����Ӧ����WIN��Ϣ��Ӧ�ļ�¼
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
	e_client_userdef_prot_code_chatroom_notify_member_info = 22048,		/// cbm(δʵ��) WM_GLB_IM2UI_CHATROOM_NOTIFY_MEMBER_INFO

	e_client_userdef_prot_code_exchange_user_person_data = 22057,		/// t2t WM_GLB_IM2UI_EXCHANGE_USER_PERSON_DATA

	e_client_userdef_prot_code_tribe_send_file = 22063,					/// tbm WM_GLB_IM2UI_TRIBE_SEND_FILE
	e_client_userdef_prot_code_ent_send_file = 22064,					/// ebm WM_GLB_IM2UI_ENT_SEND_FILE

	/// ����һЩ�ɵ��Զ�������,δ���������, ���������...

	__eclient_userdef_code_old_end__ = aot::prot::e_prot_code_client_userdef_range_old_end,/// 22100
};

enum eclient_userdef_prot_code_new
{
	/// ע��: �����������������Զ�����Ϣ, ����Ҫ�� src\component\aot_im\udmsg_to_wmmsg.cpp �ļ�������һ����Ӧ����WIN��Ϣ��Ӧ�ļ�¼
	__eclient_userdef_code_new_begin__ = aot::prot::e_prot_code_client_userdef_range_new_begin,	/// 40001

	/* tata -->tata �Զ�����Ϣ���� */
	__eclient_userdef_code_new_t2t_begin__,
	e_client_userdef_code_new_t2t_chat_inputting,									/// �Է��������� @pkt->session_info.sender_inner_id: �Է�inner_id

	/// �����ļ������Զ�����Ϣ
	__eclient_userdef_code_new_t2t_online_file_trans_begin__,
	e_client_userdef_code_new_t2t_online_file_trans_sender_request,						/// �ļ���������
	e_client_userdef_code_new_t2t_online_file_trans_sender_cancel,						/// ���ͷ�ȡ������/��;ȡ��
	e_client_userdef_code_new_t2t_online_file_trans_sender_change_to_offline,			/// ���ͷ���Ϊ���ߴ���
	e_client_userdef_code_new_t2t_online_file_trans_recver_accept,						/// ���շ�����
	e_client_userdef_code_new_t2t_online_file_trans_recver_reject,						/// ���շ��ܾ�
	e_client_userdef_code_new_t2t_online_file_trans_recver_cancel,						/// ���շ���;ȡ��
	e_client_userdef_code_new_t2t_online_file_trans_recver_high_security_level,			/// ���շ��ļ����䰲ȫ������̫��
	__eclient_userdef_code_new_t2t_online_file_trans_end__ = __eclient_userdef_code_new_t2t_online_file_trans_begin__ + 50,

	/// Զ��Э���Զ�����Ϣ
	__eclient_userdef_code_new_t2t_rd_begin__,
	e_client_userdef_code_new_t2t_rd_invite,						/// rds -> rdc: ����Զ��Э������
	e_client_userdef_code_new_t2t_rd_invite_ack,					/// rdc -> rds: ����/�ܾ� Զ��Э��
	e_client_userdef_code_new_t2t_rd_addr_info,						/// rdc -> rds: �����Լ��ĵ�ַ���Է�
	e_client_userdef_code_new_t2t_rd_addr_info_ack,					/// rds -> rdc: �����Լ��ĵ�ַ���Է�
	e_client_userdef_code_new_t2t_rd_meeting,						/// rdc -> rds: Զ��Э���ѽ�������,�����Բ鿴�Է���Ļ
	e_client_userdef_code_new_t2t_rd_enable_input_invite,			/// rds -> rdc: �������
	e_client_userdef_code_new_t2t_rd_enable_input_invite_ack,		/// rdc -> rds: ����/�ܾ� ����
	e_client_userdef_code_new_t2t_rd_disable_input_invite,			/// ˫������:   �ͷſ�������
	e_client_userdef_code_new_t2t_rd_disable_input_invite_ack,		/// ˫������:   �ͷſ���Ӧ��
	e_client_userdef_code_new_t2t_rd_reset,							/// ˫������:   ����Զ��Э��(����������ʾ)
	e_client_userdef_code_new_t2t_rd_abort,							/// ˫������:	�Ͽ�Զ��Э��
	__eclient_userdef_code_new_t2t_rd_end__ = __eclient_userdef_code_new_t2t_rd_begin__ + 50,

	/// ���ڶ����Զ�����Ϣ
	e_client_userdef_code_new_t2t_window_shake,
	/// �Զ��ظ��Զ�����Ϣ
	e_client_userdef_code_new_t2t_auto_reply,
	/// ���������ļ�����
	e_client_userdef_code_new_t2t_offline_file_trans,

	__eclient_userdef_code_new_t2t_end__ = __eclient_userdef_code_new_t2t_begin__ + 200,

	/// ebm �Զ�����Ϣ����
	__eclient_userdef_code_new_ebm_begin__,
	__eclient_userdef_code_new_ebm_end__ = __eclient_userdef_code_new_ebm_begin__ + 100,

	/// tbm �Զ�����Ϣ����
	__eclient_userdef_code_new_tbm_begin__,
	__eclient_userdef_code_new_tbm_end__ = __eclient_userdef_code_new_tbm_begin__ + 100,

	/* �ҵ��豸 -> �ҵ��豸 �Զ�����Ϣ���� */
	__eclient_userdef_code_new_d2d_begin__,
	e_client_userdef_code_new_d2d_msg_viewed,							/// ֪ͨ�ҵ������豸ĳ������Ϣ�Ѿ��鿴����
	e_client_userdef_code_new_d2d_add_buddy_ret,						/// ֪ͨ�ҵ������豸�Ѿ�ͬ��/�ܾ��Է�����������
	e_client_userdef_code_new_d2d_delete_buddy,							/// ֪ͨ�ҵ������豸�Ѿ�ɾ��������
	e_client_userdef_code_new_d2d_join_tribe_ret,						/// ֪ͨ�ҵ������豸�Ѿ�ͬ��/�ܾ�����Ⱥ����
	e_client_userdef_code_new_d2d_add_member_to_temp_tribe,				/// ֪ͨ�ҵ������豸������������Ա
	e_client_userdef_code_new_d2d_create_tribe_ret,						/// ֪ͨ�ҵ������豸������Ⱥ��
	e_client_userdef_code_new_d2d_delete_tribe_member,					/// ֪ͨ�ҵ������豸ɾ����ĳ����Ա
	e_client_userdef_code_new_d2d_quit_tribe,							/// ֪ͨ�ҵ������豸�˳���ĳ��Ⱥ
	e_client_userdef_code_new_d2d_delete_tribe,							/// ֪ͨ�ҵ������豸��ɢ��ĳ��Ⱥ
	e_client_userdef_code_new_d2d_quit_temp_tribe,						/// ֪ͨ�ҵ������豸�˳���ĳ��������
	e_client_userdef_code_new_d2d_change_tribe_member_pow,				/// ֪ͨ�ҵ������豸ָ��/������ĳ����Ա������Ա
	e_client_userdef_code_new_d2d_create_temp_tribe_ret,				/// ֪ͨ�ҵ������豸������������
	e_client_userdef_code_new_d2d_add_buddy_group_ret,					/// ֪ͨ�ҵ������豸����˺��ѷ���
	e_client_userdef_code_new_d2d_change_buddy_group,					/// ֪ͨ�ҵ������豸�޸��˺��ѷ�����
	e_client_userdef_code_new_d2d_delete_buddy_group,					/// ֪ͨ�ҵ������豸ɾ���˺��ѷ���
	e_client_userdef_code_new_d2d_move_buddy_to_group,					/// ֪ͨ�ҵ������豸�ƶ��˺��ѵ���������
	__eclient_userdef_code_new_d2d_end__ = __eclient_userdef_code_new_d2d_begin__ + 200,

	/// �ض���ͨ������Ϣ����
	__eclient_userdef_code_new_chat_msg_begine__,
	/// �����Խ�
	eclient_userdef_code_new_chat_msg_inerphone_send_sound,
	/// ����λ��
	eclient_userdef_code_new_chat_msg_location_map,
	__e_client_userdef_code_new_chat_msg_end__ = __eclient_userdef_code_new_chat_msg_begine__ + 100,

	/// end pos
	__eclient_userdef_code_new_end__ = aot::prot::e_prot_code_client_userdef_range_new_end,		/// 41000
};

}} /// end namespace aot/prot

#endif /// __AOT_PROT_CODE_CLEINT_USERDEF_H__