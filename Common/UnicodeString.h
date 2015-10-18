

#ifndef UNICODESTRING_H
#define UNICODESTRING_H

typedef enum _UNICODE_LANG_ID 
{
	LANG_HEBREW,
	LANG_CNT
}UNICODE_LANG_ID;

#define HEBREW_STR_HEIGHT	15
typedef enum _HEBREW_STR_ID
{
		HS_ID_CHINESE,//����
		HS_ID_GATEOVERTIMEALARM,//���ų�ʱ����
		HS_ID_INVALIDCARDSWIPEALARM,//��Чˢ������
		HS_ID_INFRAREDALARM,//���ⱨ��
		HS_ID_ALARMCANCEL,//������ȡ��
		HS_ID_ACCESSSYSSETTINGS,//�Ž�ϵͳ����
		HS_ID_CARDMANAGE,//������
		HS_ID_DOORPARASETTINGS,//�Ų�������
		HS_ID_OPENMODESETTINGS,//����ģʽ����
		HS_ID_ADDRESIDENTCARD,//����ס����
		HS_ID_ADDPATROLCARD,//����Ѳ����
		HS_ID_ADDAUTHCARD,//������Ȩ��
		HS_ID_DELECARD,//ɾ����
		HS_ID_BYCARD,//����ʽ
		HS_ID_BYROOM,//����ŷ�ʽ
		HS_ID_BYLOCNUM,//���ر�ŷ�ʽ
		HS_ID_OPENOVERTIME,//���ų�ʱ
		HS_ID_INVALIDCARD,//��Чˢ��
		HS_ID_MANUALSWITCH,//�ֶ�����
		HS_ID_INFRAREDALARM_2,//���ⱨ��
		HS_ID_RESCODEPSW,//�����+����
		HS_ID_CARDPSW,//��+����
		HS_ID_CARD,//��
		HS_ID_GATEOVERTIMETIMEMODIED,//���ų�ʱʱ�����޸�
		HS_ID_GATEOVERTIMEALARMISENABLED,//���ų�ʱ����������
		HS_ID_GATEOVERTIMEALARMISDISABLED,//���ų�ʱ�����ѹر�
		HS_ID_GATEOVERTIMECONTACTOROPENED,//���ų�ʱ���㿪
		HS_ID_GATEOVERTIMECONTACTORCLOSED,//���ų�ʱ�����
		HS_ID_ALARMON,//��������
		HS_ID_ALARMOFF,//�����ر�
		HS_ID_NFRAREDTIMEMODIED,//������ʱ�����޸�
		HS_ID_INFRAREDALARMISENABLED,//���ⱨ��������
		HS_ID_INFRAREDALARMISDISABLED,//���ⱨ���ѹر�
		HS_ID_INFRAREDCONTACTOROPENED,//���ⴥ�㿪
		HS_ID_INFRAREDCONTACTORCLOSED,//���ⴥ���
		HS_ID_ALARMON_2,//��������
		HS_ID_ALARMOFF_2,//�����ر�
		HS_ID_CONTACTORON,//������
		HS_ID_CONTACTOROFF,//�մ���
		HS_ID_GATEPULSEWIDTHMODFYED,//�Ŵ����������޸�
		HS_ID_INVALIDSWIPETIMEMODIED,//��Чˢ���������޸�
		HS_ID_INVALIDSWIPEALARMISENABLED,//��Чˢ������������
		HS_ID_INVALIDSWIPEALARMISDISABLED,//��Чˢ�������ѹر�
		HS_ID_ALARMON_3,//��������
		HS_ID_ALARMOFF_3,//�����ر�
		HS_ID_MANOPENENABLED,//�ֶ�����������
		HS_ID_MANOPENDISABLED,//�ֶ������ѹر�
		HS_ID_ENTERFUNNUM,//�����빦����
		HS_ID_PRESSENTER,//��#��ȷ��
		HS_ID_CONFDELETE,//ȷ��ɾ����
		HS_ID_PRESSHELP,//��#�鿴����
		HS_ID_NETFAIL,//�������
		HS_ID_ENTERRESNUM,//�����뷿��� ��#��ȷ��
		HS_ID_ENTERCALLOCODE,//�����뷿��� 
		HS_ID_CALLOUT,//���ں���
		HS_ID_MC,//��������
		HS_ID_CONVERTIME,//ͨ���С�
		HS_ID_REMAINING,//Remaining��
		HS_ID_SECONDS,//��
		HS_ID_MCCALLIN,//�����������ں���
		HS_ID_CGMCALLIN,//���Ż����ں���
		HS_ID_HVCALLIN,//���ڻ����ں���
		HS_ID_PRESSANSWER,//��*����
		HS_ID_LINEBUSY,//��·æ
		HS_ID_TRYAGAIN,//���Ժ�����
		HS_ID_CALLEND,//ȡ������
		HS_ID_CONVEREND,//ͨ������
		HS_ID_NOANSWER,//����Ӧ�� 
		HS_ID_ANSWERLATER,//���Ժ��ٺ�
		HS_ID_IMGCAP,//������ͼ
		HS_ID_NOTREGISTER,//�����δע��
		HS_ID_GATEUNLOCK,//�����ɹ�
		HS_ID_PATROLCARD,//Ѳ����
		HS_ID_IPADDRESS,//IP��ַ:
		HS_ID_PHYADDRESS,//�����ַ:
		HS_ID_ENTERROR,//��������
		HS_ID_FUNISNOTEN,//����δ��ͨ
		HS_ID_CARDISNOTAU,//��δ��Ȩ
		HS_ID_AUTHORIZEDCARD,//��Ȩ��
		HS_ID_CARDISNOTENABLED,//��������
		HS_ID_CARDEXPIRED,//��������Ч��
		HS_ID_ETRPWD,//���������� ��#��ȷ��
		HS_ID_ETRULKPWD,//�����뿪������
		HS_ID_ETRPATRULKPWD,//Ѳ����,�����뿪������
		HS_ID_PWDINCORRECT,//�������
		HS_ID_SWIPECARD,//��ˢ��
		HS_ID_ENTEROLDPSW,//����������� ��#��ȷ��
		HS_ID_OLDPSWERR,//���������
		HS_ID_ENTERNEWPSW,//������������
		HS_ID_PSWDIGITERROR,//����λ������
		HS_ID_ENTERNEWPSWAG,//���ٴ�����������
		HS_ID_PSWDIFF,//���벻һ��
		HS_ID_INPUTAG,//����������
		HS_ID_PSWMODIFYOK,//�����޸ĳɹ�
		HS_ID_PRESSEXIT,//��*�˳�
		HS_ID_CARDHASREG,//����ע��
		HS_ID_CARDREGISTERED,//��ע��ɹ�
		HS_ID_ENTERLOCID,//�����뱾�ر�� ��#��ȷ��
		HS_ID_IDHASREG,//ID����ע��
		HS_ID_RDNUMNTEX,//����Ų�����
		HS_ID_CARDDELETED,//��ɾ���ɹ�
		HS_ID_CARDHASDELETED,//����ɾ��
		HS_ID_TEST101,//�����뱾�ر�� ��#��ȷ��
		HS_ID_ID_NOTREGISTERED,//ID��δע��
		HS_ID_ETRVLDSTATIME,//��������Ч��ʼ���� ��#��ȷ��
		HS_ID_ETRVLDENDTIME,//��������Ч��ֹ���� ��#��ȷ��
		HS_ID_ETRSYSPWD,//������ϵͳ���� ��#��ȷ��
		HS_ID_ROOMCODE_NO_CARD,//�����δע�Ῠ
		HS_ID_CALLPRESSCODE,//����ס����ֱ�Ӱ������
		HS_ID_CALLPRESS00,//���й��������밴00
		HS_ID_TEST109,//����
		HS_ID_ENGLISH_SELED,//������ѡ��
		HS_ID_SYSSETTINGS,//ϵͳ����
		HS_ID_TALKSETTINGS,//ͨ������
		HS_ID_ENTRYSETTINGS,//��������
		HS_ID_ACCESSSETTINGS,//�Ž�����
		HS_ID_SYSINFO,//ϵͳ��Ϣ
		HS_ID_RESETSETTINGS,//��������
		HS_ID_IP,//IP��ַ
		HS_ID_NETMASK,//��������
		HS_ID_IPMODIFYED,//IP���޸�
		HS_ID_PLEASERESTART,//������
		HS_ID_NETMASKMODIFYED,//�����������޸�
		HS_ID_RINGTIMEMODFYED,//����ʱ�����޸�
		HS_ID_MODIFYED,//�������Чλ���޸�
		HS_ID_LANGUAGE,//����
		HS_ID_NETWORK,//����
		HS_ID_MODIFYPSW,//�޸�����
		HS_ID_TAMPERALARM,//���𱨾�
		HS_ID_TEST128,//Ӳ���汾��
		HS_ID_TEST129,//����汾��
		HS_ID_TEST130,//��Ʒ���
		HS_ID_VALIDROOMNUM,//�������Чλ
		HS_ID_TALKTIME,//ͨ��ʱ��
		HS_ID_RINGTIME,//����ʱ��
		HS_ID_SURE,//ȷ����?
		HS_ID_CONFIRM,//��#ȷ��
		HS_ID_PRESSOTHER,//���������˳�
		HS_ID_RESTORE,//��#ϵͳ��ԭ
		HS_ID_PRESSOTHER2,//������������
		HS_ID_TALKTIMEMODED,//ͨ��ʱ�����޸�
		HS_ID_INVALIDCARDSWIPEALARM_2,//��Чˢ������
		HS_ID_TAMPERALARM_2,//���𱨾�
		HS_ID_GATEOVERTIMEALARM_2,//���ų�ʱ����
		HS_ID_INFRAREDALARM_3,//���ⱨ��
		HS_ID_TAMPER_OPEN_ALARM,//����+���ų�ʱ����
		HS_ID_TAMPER_INFRARED_ALARM,//����+���ⱨ��
		HS_ID_OPEN_INFRARED_ALARM,//���ų�ʱ+���ⱨ��
		HS_ID_ALARM_ALL,//����+����+���ⱨ��
		HS_ID_ALARMCONFIRM,//������ȷ��
		HS_ID_TEST149,//hebrew
		HS_ID_HEBREW_SELED,//hebrew selected
		HS_ID_LANG_CHINESE,//Chinese
		HS_ID_LANG_ENGLISH,//English
		HS_ID_LANG_HEBREW,//Hebrew
		HS_ID_LANG_CHINESE_SELED,//Chinese Selected
		HS_ID_LANG_RESTART_CN,
		HS_ID_ON,//����
		HS_ID_OFF,//close
		HS_ID_INVALID_IP,//Invalid IP
		HS_ID_CNT,
}HEBREW_STR_ID;

#endif


