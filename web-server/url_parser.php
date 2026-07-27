<?php

$rootdir="/var/www/html/trainingplanner/";
$pause_file=$rootdir . "pause";
$htpasswd_file=$rootdir . "admin/.passwds";

if (file_exists($pause_file)) {
	$paused = file($pause_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	if ($paused[0] == "1") {
		exit(get_return_code("server paused") . ": TrainingPlanner app server under maintenance!", 1);
	}
}

include_once('tp_functions.php');

/* Can be used by either an ordinary user or admin.
 * Since they are all file operations, they follow the same rationale applied to a TPFilePath:
 * $owner_user / $target_user / $subdir / $filename
 * $owner_user is obligatory, as well as $filename.
 **/
function parseFileQuery($userid): array {
	if (!isset($_GET['owner']))
		return array(false, get_return_code("argument missing") . ": Missing *owner* argument");
	global $rootdir;
	$owner_user = $_GET['owner'];
	$filedir = "$rootdir$owner_user/";
	if (isset($_GET['target'])) {
		$target_user = $_GET['target'];
		$filedir .= $target_user;
	} else {
		$target_user = "";
	}
	if (isset($_GET['subdir'])) {
		$subdir = $_GET['subdir'];
		$filedir .= $subdir;
	} else {
		$subdir = "";
	}

	if (isset($_GET['listfiles']) || isset($_GET['listdirs'])) {
		if ($userid != "admin") {
			if ($target_user) {
				if ($target_user !== $userid)
					return array(false, get_return_code("no privilege") . ": Regular users cannot list files from other users");
			}
		}
		$pattern = isset($_GET['pattern']) ? $_GET['pattern'] : '';
		$recursive = isset($_GET['recursive']);
		$files = scan_dir($filedir, $pattern, isset($_GET['listfiles']), isset($_GET['listdirs']), true, $recursive, $owner_user);
		$res = true;
		$msg = "0: $files";
	} elseif (isset($_GET['upload'])) {
		//When uploading a file to another user, set $targetuser. The sender can only send files to its subdirectory within $targetuser
		if ($userid != "admin") {
			$backupdir = "";
			if ($target_user) {
				if ($target_user !== $userid)
					return array(false, get_return_code("no privilege") . ": $userid can only send files to itself or *target* must be set to $userid");
				else //make a copy of the sent file in the user's directory for recovery and/or syncing
					$backupdir = "$rootdir$userid/$target_user/$subdir";
			}
		}
		return upload_file($filedir, $backupdir);
	} elseif (isset($_GET['checkfilectime'])) {
		return check_file_ctime($filedir . '/' . $_GET['checkfilectime']);
	} else {
		//A user can only get/delete files from within their base directory. Admin can get/delete any file
		if ($userid != "admin") {
			if ($owner_user !== $userid)
				return array(false, get_return_code("no privilege") . ": $userid can only get/delete files in its own directory");
		}
		if (isset($_GET['file'])) {
			return download_file($_GET['file'], $filedir);
		} elseif (isset($_GET['delfile'])) {
			$res = false;
			$file = $filedir . '/' . $_GET['delfile'];
			if (is_file($file)) {
				$res = unlink($file);
				$msg = $res === true ? "0: $file deleted" : get_return_code("delete failed") . ": Failed to delete $file";
			} else {
				$msg = get_return_code("file not found") . ": cannot delete $file because it does not exist";
			}
		} else {
			$msg = get_return_code("argument missing") . ": Missing action argument at: " . __LINE__;
		}
	}
	return array($res, $msg);
}

function parseAdminQuery(): array {
	$result = array(1, "");
	if (isset($_GET['checkaccount'])) {
		$query = $_GET['checkaccount'];
		//Check if there is an already existing user in the online database.
		//The unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
		$userpassword = isset($_GET['userpassword']) ? $_GET['userpassword'] : '';
		$result = run_dbscript("getid", $query . " '" . $userpassword . "'", "");
	} elseif (isset($_GET['allusers'])) {
		$all_users = scan_dir($rootdir, "", true, false, false, false);
		$users = remove_from_string($all_users, "admin|");
		$users = remove_from_string($users, "scripts|");
		$result[0] = 0;
		$result[1] = "0: $users";
	} elseif (isset($_GET['onlinedata'])) {
		//Check if there is an already existing user in the online database. The  unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
		$result = run_dbscript("getall", "", $_GET['onlinedata']);
	} elseif (isset($_GET['adduser'])) {
		//new user creation. Encrypt password onto file and create the user's dir
		$user_id = $_GET['adduser'];
		$new_user_password = isset($_GET['userpassword']) ? $_GET['userpassword'] : '';
		$result = user_exists($user_id, $new_user_password);
		if ($result[0] === 6) {
			$result = run_htpasswd("-bB", $user_id, $new_user_password);
			if ($result[0] === 0) {
				$userdir = $rootdir . $user_id;
				create_dir("$userdir/Database"); //Creates the user dir and its database subdir
				create_dir("$userdir/chats"); //Creates the user dir and its database subdir
				if (update_datafile_with_password($user_id))
					$result = run_dbscript("add", "", $user_id);
			}
		}
	} elseif (isset($_GET['deluser'])) {
		//remove user and their dir
		$user_id = $_GET['deluser'];
		if (!isset($_GET['userpassword'])) {
			$result[1] = get_return_code("password missing") . ": Cannot update user information. No user password provided.";
		} else {
			$user_password = $_GET['userpassword'];
			$result = user_exists($user_id, $new_user_password);
			if ($result[0] === 0) {
				del_coach($user_id);
				$result = run_htpasswd("-D", $user_id, "");
				if ($result[0] === 0) {
					$result = run_dbscript("del", "", $user_id);
					$userdir = $rootdir . $user_id;
					erasedir($userdir);
				}
			}
		}
	}
	$result[0] = $result[0] === 0 ? true : false;
	return $result;
}

function parseUserQuery($userid): array {
	global $rootdir;
	$result = array(1, "");
	if (isset($_GET['runcmds'])) {
		$result = run_commands($userid, $_GET['runcmds'], true);
	} elseif (isset($_GET['downloadcmd'])) {
		$subdir = isset($_GET['subdir']) ? $_GET['subdir'] . "/" : '';
		$filedir = $rootdir . $userid . "/" . $subdir;
		$device_id = $_GET['deviceid'];
		if ($device_id == "")
			$result[1] = get_return_code("no device id") . ": Missing device id argument **downloadcmd**";
		elseif (!is_device_listed($userid, $device_id))
			$result[1] = get_return_code("deviceid not registered") . ": Device id not registered **downloadcmd**";
		else {
			$result = download_file($_GET['downloadcmd'], $filedir);
			if ($result[0] === 0)
				$result = cmd_downloaded($userid, $device_id, $filedir . $_GET['downloadcmd']);
		}
	} elseif (isset($_GET['login'])) {
		$port = $_GET['login'];
		$device_id = $_GET['device'];
		add_device($userid, $device_id);
		set_online_visible($userid, true);
		clear_apcu_cache($userid);
		apcu_store($userid."tpmessages", true); //$has_new_messages = true
		apcu_store($userid, $_SERVER['REMOTE_ADDR'] . ":" .$port);
		$result[0] = true;
		$result[1] = "0: User $userid logged in";
	} elseif (isset($_GET['logout'])) {
		$device_id = $_GET['device'];
		#del_device($userid, $device_id);
		set_online_visible($userid, false);
		clear_apcu_cache($userid);
		$result[0] = true;
		$result[1] = "0: User $userid logged out";
	} elseif (isset($_GET['getpeeraddress'])) {
		$peer = $_GET['getpeeraddress'];
		$peer_addr = apcu_fetch($peer);
		if ($peer_addr == false)
			$result[1] = get_return_code("user does not exist") . ": $peer is not logged in";
		else {
			$result[0] = true;
			$result[1] = "0: $peer_addr";
		}
	} elseif (isset($_GET['getdeviceslist'])) {
		$result = get_devices_list($userid);
	} else if (isset($_GET['changepassword'])) {
		$old_password = $_GET['changepassword'];
		$result = user_exists($userid, $old_password);
		if ($result[0] === 0) {
			$new_password = $_GET['newpassword'];
			if ($new_password == "") {
				$result[0] = 1;
				$result[1] = get_return_code("argument missing") . ": no new passord given **changecredentials**";
			} else {
				$result = run_htpasswd("-bB", $userid, $new_password);
				if ($result[0] === 0)
					$result[1] = "0: $userid password successfully modified";
				else
					$result[1] = get_return_code("unknown error") . ": $userid password modification failed";
			}
		}
	} else if (isset($_GET['addcoach'])) {
		$result = add_coach($userid);
	} elseif (isset($_GET['delcoach'])) {
		$result = del_coach($userid);
	} elseif (isset($_GET['getonlinecoaches'])) {
		$result = get_online_coaches();
	} elseif (isset($_GET['requestcoach'])) {
		$coach = $_GET['requestcoach'];
		$result = request_coach($userid, $coach);
	} elseif (isset($_GET['deleteclientrequest'])) {
		$client = $_GET['deleteclientrequest'];
		$result = delete_client_request($userid, $client);
	} elseif (isset($_GET['listclientsrequests'])) {
		$result = list_clients_requests($userid);
	} elseif (isset($_GET['acceptclientrequest'])) {
		$client = $_GET['acceptclientrequest'];
		$result = accept_client_request($userid, $client);
	} elseif (isset($_GET['rejectclientrequest'])) {
		$client = $_GET['rejecttclientrequest'];
		$result = reject_client_request($userid, $client);
	} elseif (isset($_GET['deletecoachanswer'])) {
		$coach = $_GET['deletecoachanswer'];
		$result = delete_coach_answer($userid, $coach);
	} elseif (isset($_GET['listcoachesanswers'])) {
		$result = list_coaches_answers($userid);
	} elseif (isset($_GET['acceptcoachanswer'])) {
		$coach = $_GET['acceptcoachanswer'];
		$result = accept_coach_answer($userid, $coach);
	} elseif (isset($_GET['rejectcoachanswer'])) {
		$coach = $_GET['rejectcoachanswer'];
		$result = reject_coach_answer($userid, $coach);
	} elseif (isset($_GET['getclients'])) {
		$result = get_clients_list($userid);
	} elseif (isset($_GET['removecurclient'])) {
		$client = $_GET['removecurclient'];
		$result = remove_client_from_clients($userid, $client);
	} elseif (isset($_GET['getcoaches'])) {
		$result = get_coaches_list($userid);
	} else if (isset($_GET['removecurcoach'])) {
		$coach = $_GET['removecurcoach'];
		$result = remove_coach_from_coaches($userid, $coach);
	} elseif (isset($_GET['gettpmessages'])) {
		$result = get_tpmessages($userid);
	} elseif (isset($_GET['sendtpmessage'])) {
		$receiver = $_GET['sendtpmessage'];
		$message = $_GET['message'];
		if ($message == "")
			$result[1] = get_return_code("argument missing") . ": No message argument **sendtpmessage**";
		else
			$result = send_tpmessage($receiver, $message);
	} elseif (isset($_GET['removetpmessage'])) {
		$message = $_GET['removetpmessage'];
		if ($message == "")
			$result[1] = get_return_code("argument missing") . ": No message argument **removetpmessage**";
		$result = remove_tpmessage($userid, $message);
	} elseif (isset($_GET['getnewchatmessages'])) {
		$result = get_newchatmessages($userid);
	} elseif (isset($_GET['sendchatmessage'])) {
		$receiver=$_GET['sendchatmessage'];
		if ($receiver == "")
			$result[1] = get_return_code("argument missing") . ": No receiver argument **sendchatmessage**";
		$message = $_GET['message'];
		if ($message == "")
			$result[1] = get_return_code("argument missing") . ": No message argument **sendchatmessage**";
		$result = send_chatmessage($userid, $receiver, $message);
	} elseif (isset($_GET['removechatmessage'])) {
		$receiver=$_GET['removechatmessage'];
		if ($receiver == "")
			$result[1] = get_return_code("argument missing") . ": No receiver argument **removechatmessage**";
		$message = $_GET['message'];
		if ($message == "")
			$result[1] = get_return_code("argument missing") . ": No message argument **removechatmessage**";
		$result = remove_chat_message($userid, $receiver, $message);
	} elseif (isset($_GET['forcegetnewmessages'])) {
		clear_apcu_cache($userid);
	} else {
		$result[1] = get_return_code("argument missing") . ": Missing action argument at: " . __LINE__;
	}
	$result[0] = $result[0] === 0 ? true : false;
	return $result;
}

function main(): void {
	$result = array(false, "");
	$userid = isset($_GET['user']) ? $_GET['user'] : '';
	if ($userid) {
		$password = isset($_GET['password']) ? $_GET['password'] : '';
		if (!$password) {
			$result[1] = get_return_code("password missing") . ": Missing password";
		} else {
			global $htpasswd_file;
			$result = verify_credentials($userid, $password, $htpasswd_file);
			if ($result[0] === true) {
				// Authentication successful
				if (isset($_GET['test'])) {
					$result = run_test_function($userid, $password);
				} else {
					$result = parseFileQuery($userid);
					if (str_starts_with($result[1], get_return_code("argument missing"))) {
						if ($userid === "admin")
							$result = parseAdminQuery();
						else
							$result = parseUserQuery($userid);
					}
				}
			} else {
				// Authentication failed
				header('HTTP/1.1 401 Unauthorized');
				if ($result[1] === "") {
					$return_var = user_exists($target_user, $password);
					$result = user_exists_return_message($return_var);
				}
			}
		}
	} else {
		$result[0] = true;
		$result[1] = "Welcome to the TrainingPlanner app server!";
	}
	echo $result[1];
	exit($result[0]);
}

main();

?>
