/*
 * msg.c
 *
 *  Created on: 2010/05/06
 *      Author: takka
 */

// MENU
char *MSG_NULL[2] = {
    "\0",
    "\0"
};

char *MSG_SEPARATE[2] = {
    "\t",
    "\t"
};

char *MSG_UPDATE_UMD_ID_CSV[2] = {
    "UMD_ID.csv更新",
    "Update UMD_ID.csv"
};

char *MSG_UPDATE_ISO_TOOL[2] = {
    "iso_tool更新",
    "Update iso_tool"
};

char *MSG_UPDATE_PSPDECRYPT_PRX[2] = {
    "pspdecrypt.prx更新",
    "Update pspdecrypt.prx"
};

char *MSG_REBOOT[2] = {
  "再起動",
  "Reboot"
};

char *MSG_EXIT[2] = {
  "終了",
  "Exit"
};

char *MSG_EBOOT_DECRYPTION[2] = {
    "EBOOT 復号",
    "EBOOT decrypt"
};

char *MSG_RECOVERY_EBOOT[2] = {
    "EBOOTリカバリ",
    "Recovery EBOOT"
};

char *MSG_RENAME[2] = {
    "リネーム",
    "Rename"
};

char *MSG_ISO_CONVERT[2] = {
    "ISO変換",
    "ISO Convert"
};

char *MSG_CSO_CONVERT[2] = {
    "CSO変換",
    "CSO Convert"
};

char *MSG_PATCH_KERNEL_LIBRARO[2] = {
    "Kernel_LibrarOパッチ",
    "Patch Kernel_LibrarO"
};

char *MSG_PATCH_KERNEL_LIBRARZ[2] = {
    "Kernel_LibrarZパッチ",
    "Patch Kernel_LibrarZ"
};

char *MSG_PATCH_PROMETHEUS[2] = {
    "Prometheusパッチ",
    "Patch Prometheus"
};

char *MSG_RECOVERY_PROMETHEUS[2] = {
    "Prometheus復旧",
    "RECOVERY Prometheus"
};

char *MSG_DELETE[2] = {
    "削除",
    "Delete"
};

char *MSG_UNIMPLEMENTED[2] = {
    "未実装",
    "Unimplemented"
};

char *MSG_YES[2] = {
    "はい",
    "Yes"
};

char *MSG_NO[2] = {
    "いいえ",
    "No"
};

char *MSG_OSK[2] = {
    "OSK",
    "OSK"
};

char *MSG_UMD_ID[2] = {
    "UMD ID",
    "UMD ID"
};

char *MSG_JPANESE[2] = {
    "日本語名",
    "Japanese"
};

char *MSG_ENGLISH[2] = {
    "英語名",
    "English"
};

char *MSG_0[2] = {
    "0",
    "0"
};

char *MSG_1[2] = {
    "1",
    "1"
};

char *MSG_2[2] = {
    "2",
    "2"
};

char *MSG_3[2] = {
    "3",
    "3"
};

char *MSG_4[2] = {
    "4",
    "4"
};

char *MSG_5[2] = {
    "5",
    "5"
};

char *MSG_6[2] = {
    "6",
    "6"
};

char *MSG_7[2] = {
    "7",
    "7"
};

char *MSG_8[2] = {
    "8",
    "8"
};

char *MSG_9[2] = {
    "9",
    "9"
};

char *MSG_100[2] = {
    "100%",
    "100%"
};

char *MSG_95[2] = {
    " 95%",
    " 95%"
};

char *MSG_90[2] = {
    " 90%",
    " 90%"
};

char *MSG_85[2] = {
    " 85%",
    " 85%"
};

char *MSG_80[2] = {
    " 80%",
    " 80%"
};

char *MSG_75[2] = {
    " 75%",
    " 75%"
};

char *MSG_70[2] = {
    " 70%",
    " 70%"
};

char *MSG_65[2] = {
    " 65%",
    " 65%"
};

char *MSG_60[2] = {
    " 60%",
    " 60%"
};

char *MSG_55[2] = {
    " 55%",
    " 55%"
};

char *MSG_50[2] = {
    " 50%",
    " 50%"
};

char *MSG_BACKUP[2] = {
    "バックアップ",
    "Backup"
};

char *MSG_BACKUP_TEXT[2] = {
    "EBOOT.BINのバックアップを保存しますか?",
    "Do you want to store a backup of EBOOT.BIN?"
};

char *MSG_VERSION_PATCH[2] = {
    "バージョン パッチ",
    "Version Patch"
};

char *MSG_VERSION_PATCH_TEXT[2] = {
    "バージョンパッチ機能を有効にしますか?",
    "Do you want to enable the Version Patch?"
};

char *MSG_START[2] = {
    "開始",
    "Start"
};

char *MSG_START_TEXT[2] = {
    "開始しますか?",
    "Do you want to start?"
};

char *MSG_COMPRESSION_LEVEL[2] = {
    "圧縮レベル",
    "Compression level"
};

char *MSG_COMPRESSION_LEVEL_TEXT[2] = {
    "圧縮レベル 1(低圧縮)～9(高圧縮)",
    "Compression level 1(Low)～9(High)"
};

char *MSG_THRESHOLD[2] = {
    "閾値",
    "Threshold"
};

char *MSG_THRESHOLD_TEXT[2] = {
    "閾値 55%(低圧縮)～100%(高圧縮)",
    "Threshold 55%(Low)～100%(High)"
};

char *MSG_CHANGED_KERNEL_LIBRARO[2] = {
    "Kernel_LibrarOに変更しました",
    "Changed to Kernel_LibrarO."
};

char *MSG_CHANGED_KERNEL_LIBRARZ[2] = {
    "Kernel_LibrarZに変更しました",
    "Changed to Kernel_LibrarZ."
};

char *MSG_CHANGED_KERNEL_LIBRARY[2] = {
    "Kernel_Libraryに変更しました",
    "Changed to Kernel_Library."
};

char *MSG_CHANGED_SCEUTILITO[2] = {
    "sceUtilitOに変更しました",
    "Changed to sceUtilitO."
};

char *MSG_CHANGED_SCEUTILITY[2] = {
    "sceUtilityに変更しました",
    "Changed to sceUtility."
};

char *MSG_READING_EBOOT_BIN[2] = {
    "EBOOT.BINを読込んでいます",
    "Reading EBOOT.BIN"
};

char *MSG_WRITING_EBOOT_BIN[2] = {
    "EBOOT.BINを書込んでいます",
    "Writing EBOOT.BIN"
};

char *OBJECT_NOT_FOUND[2] = {
    "対象が見つかりませんでした",
    "Object not found."
};

char *MSG_FINISHED[2] = {
    "完了しました",
    "Finished."
};

char *MSG_PUSH_BUTTON[2][2] = {
    { "◎を押して下さい", "○を押して下さい" },
    { "Push ◎ button.", "Push ○ button." }
};

char *MSG_ALREADY_DECRYPT_EBOOT_BIN[2] = {
    "EBOOT.BINは復号済みです",
    "Already decrypt EBOOT.BIN"
};

char *MSG_DECRYPTING_EBOOT_BIN[2] = {
    "EBOOT.BINを復号しています",
    "Decrypting EBOOT.BIN"
};

char *MSG_PATCHED_EBOOT_BIN[2] = {
    "EBOOT.BINにパッチをあてました",
    "Patched EBOOT.BIN"
};

char *MSG_RECOVERING_EBOOT_BIN[2] = {
    "EBOOT.BINをリカバリしています",
    "Recovering EBOOT.BIN"
};

char *MSG_TYPE[2] = {
    "種類",
    "Type"
};

char *MSG_FILE_NAME[2] = {
    "ファイル名",
    "File Name"
};

char *MSG_OUTPUT_NAME[2] = {
    "出力名：%s",
    "Output name : %s"
};

char *MSG_THERE_ARE_FILES_WITH_THE_SAME_NAME[2] = {
    "同名のファイルがあります",
    "There are files with the same name."
};

char *MSG_OVERWRITE_[2] = {
    "上書きしますか？",
    "Overwrite?"
};

char *MSG_OVERWRITE[2] = {
    "上書き",
    "Overwrite"
};

char *MSG_UMD_ISO_CONVERT[2] = {
    "UMD -> ISO 変換",
    "UMD -> ISO Convert"
};

char *MSG_UMD_CSO_CONVERT[2] = {
    "UMD -> CSO 変換 (圧縮レベル:%d/閾値%3d%%)",
    "UMD -> CSO Convert (Compression:%d/Threshold%3d%%)"
};

char *MSG_ISO_CSO_CONVERT[2] = {
    "ISO -> CSO 変換 (圧縮レベル:%d/閾値%3d%%)",
    "ISO -> CSO Convert (Compression:%d/Threshold%3d%%)"
};

char *MSG_CSO_ISO_CONVERT[2] = {
    "CSO -> ISO 変換",
    "CSO -> ISO Convert"
};

char *MSG_SECTOR_COMPLETE[2] = {
    "%d / %d セクタ完了",
    "%d / %d Sector complete."
};

char *MSG_NOW_EST_LEFT[2] = {
    "経過 %02d:%02d:%02d / 予想 %02d:%02d:%02d / 残 %02d:%02d:%02d",
    "Now %02d:%02d:%02d / Est %02d:%02d:%02d / Left %02d:%02d:%02d"
};

char *MSG_SPEED[2] = {
    "速度 %4dKB/s",
    "Speed %4dKB/s"
};

char *MSG_STOP[2] = {
    "STOP !!",
    "Stop !!"
};

char *MSG_STOP_TEXT[2] = {
    "中止しますか？",
    "Do you want to stop?"
};

char *MSG_CONVERSION_STOP[2] = {
    "変換中止",
    "Conversion stop."
};

char *MSG_TOTAL_TIME_AVERAGE_SPEED[2] = {
    "変換時間 %02d:%02d:%02d / 平均速度 %04dKB/s",
    "Total time %02d:%02d:%02d / Average speed %04dKB/s"
};

char *MSG_NOW_EST_LEFT_FINISH[2] = {
    "経過 %02d:%02d:%02d / 予想 %02d:%02d:%02d / 残 00:00:00",
    "Now %02d:%02d:%02d / Est %02d:%02d:%02d / Left 00:00:00"
};

char *MSG_FREE_SPACE_IS_LESS_THAN_ISO_SIZE[2] = {
    "空き容量がISOサイズ以下です",
    "Free space is less than ISO size."
};

char *MSG_THERE_IS_A_POSSIBILITY_OF_FAILING[2] = {
    "途中で変換に失敗する可能性があります",
    "There is a possibility of failing."
};

char *MSG_CONTINUE[2] = {
    "続行しますか?",
    "Continue?"
};

char *MSG_CSO_CONVERSION_ERROR[2] = {
    "CSO変換エラー",
    "CSO conversion error."
};

char *MSG_UPDATE[2] = {
    "更新",
    "Update"
};

char *MSG_CONNECTION[2] = {
    "接続",
    "Connection."
};

char *MSG_UPDATE_CHECK[2] = {
    "更新確認",
    "Update check."
};

char *MSG_GET_UPDATE_TIME_ERROR[2] = {
    "更新時刻取得エラー",
    "Get update time error."
};

char *MSG_NOT_UPDATE[2] = {
    "更新されていません",
    "Not update."
};

char *MSG_FOUND_UPDATE_DO_YOU_WANT_TO_UPDATE[2] = {
    "更新が見つかりました。更新しますか？",
    "Found update. Do you want to update?"
};

char *MSG_CONFIRM[2] = {
    "確認",
    "Confirm"
};

char *MSG_GET_FILE[2] = {
    "ファイル取得",
    "Get file."
};

char *MSG_GET_FILE_ERROR[2] = {
    "ファイル取得エラー",
    "Get file error."
};

char *MSG_FILE_UPDATE[2] = {
    "ファイル更新",
    "File update."
};

char *MSG_OLD_FILE_DELETE_ERROR[2] = {
    "旧ファイル削除エラー",
    "Old file delete error."
};

char *MSG_RENAME_ERROR[2] = {
    "リネームエラー",
    "File rename error."
};

char *MSG_DISCONNECT[2] = {
    "切断",
    "Disconnect."
};

char *MSG_RELOAD_DATA[2] = {
    "データ再読込",
    "Reload data."
};

char *MSG_RESTART[2] = {
    "再起動します",
    "Restart."
};

char *MSG_DELETE_[2] = {
    "削除しますか?",
    "Delete?"
};

char *MSG_LIBFONT_PATCH[2] = {
    "libfont patch",
    "libfont patch."
};

char *MSG_LIBFONT_RECOVERY[2] = {
    "libfont 復旧",
    "libfont Recovery."
};

char *MSG_FINISHED_RECOVERY[2] = {
    "リカバリ完了しました",
    "Finished Recovery."
};

char *MSG_MENU_PATCH[2] = {
    "パッチ",
    "Patch."
};

char *MSG_MENU_PRX[2] = {
    "PRX インポート",
    "PRX Import."
};

char *MSG_IMPORT_LIBFONT[2] = {
    "libfont.prx インポート",
    "libfont.prx Import."
};

char *MSG_IMPORT_LIBPSMFPLAYER[2] = {
    "libpsmfplayer.prx インポート",
    "libpsmfplayer.prx Import."
};

char *MSG_IMPORT_PSMF[2] = {
    "psmf.prx インポート",
    "psmf.prx Import."
};

char *MSG_RECOVERY_LIBFONT[2] = {
    "libfont.prx 復旧",
    "libfont.prx Recovery."
};

char *MSG_RECOVERY_LIBPSMFPLAYER[2] = {
    "libpsmfplayer.prx 復旧",
    "libpsmfplayer.prx Recovery."
};

char *MSG_RECOVERY_PSMF[2] = {
    "psmf.prx 復旧",
    "psmf.prx Recovery."
};

char *MSG_EBOOT_WRITE[2] = {
    "EBOOT.BIN 書出し",
    "EBOOT.BIN Write."
};

char *MSG_EBOOT_IMPOERT[2] = {
    "EBOOT.BIN インポート",
    "EBOOT.BIN Import."
};

// screen
char *MSG_STATUS[2] = {
    "ステータス",
    "STATUS"
};

