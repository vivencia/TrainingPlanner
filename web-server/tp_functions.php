<?php

$rootdir="/var/www/html/trainingplanner/";
$scriptsdir=$rootdir . "scripts/";
$htpasswd_file=$rootdir . "admin/.passwds";
$coaches_file=$rootdir . "admin/coaches.data";
$htpasswd="/usr/bin/htpasswd"; //use fullpath
$developmentMode=true;

if ($developmentMode) {
	$dirMode = 0775;
	$fileMode = 0664;
}
else {
	$dirMode = 0750;
	$fileMode = 0640;
}

// set the default timezone to use.
date_default_timezone_set('America/Sao_Paulo');

function print_r2($val){
		echo '<pre>';
		print_r($val);
		echo  '</pre>';
}

function clear_apcu_cache($target_user) {
	$iter = new APCUIterator(null, APC_ITER_KEY);
	foreach ($iter as $key) {
		if (str_starts_with($iter->key(), $target_user))
			apcu_delete($iter->key());
	}
}

function get_return_code(string $desc): int {
	global $scriptsdir;
	$codes_file = $scriptsdir . "return_codes.h";
	$ret_codes = file($codes_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	foreach ($ret_codes as $ret_code) {
		if (str_contains($ret_code, str_replace(' ', '_', strtoupper($desc)))) {
			$last_field_pos = strrpos($ret_code, '	');
			if ($last_field_pos)
				return substr($ret_code, $last_field_pos + 1);
			else
				break;
		}
	}
	return 101; //Unknown error code
}

function erasedir($path): bool {
	if (!is_dir($path))
		return true;
	$dir = opendir($path);
	while( false !== ($file = readdir($dir)) ) {
		if (( $file != '.' ) && ( $file != '..' )) {
			$full = $path . '/' . $file;
			if (is_dir($full))
				return erasedir($full);
			else if (is_file($full))
				unlink($full);
		}
	}
	closedir($dir);
	return rmdir($path);
}

//!!Attention!! mkdir() does not set the permissions specified. Must use chmod() afterwards
function create_dir($directory): bool {
	global $dirMode;
	if (!is_dir($directory)) {
		if (!mkdir($directory, $dirMode, true))
			return false;
		chmod($directory, $dirMode);
	}
	return true;
}

function chper($file) {
	global $fileMode;
	chmod($file, $fileMode);
}

// Function to verify credentials against .htpasswd file
function verify_credentials($target_user, $password, $htpasswd_file): array {
	$res = false;
	$msg = "";
	if (!file_exists($htpasswd_file)) {
		$msg = "htpasswd file not found";
	} else {
		#print_r2("Authenticating " . $target_user . " with password " . $password);
		// Read the .htpasswd file line by line
		$lines = file($htpasswd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($lines as $line) {
			// Each line is in format: username:hashed_password
			list($storedUser, $storedHash) = explode(':', $line, 2);
			if ($storedUser === $target_user) {
				// Check password against stored hash
				if (password_verify($password, $storedHash) || crypt($password, $storedHash) === $storedHash) {
					$res = true;
					break;
				}
			}
		}
		$msg = "User ($target_user) not found";
	}
	return array($res, $msg);
}

function upload_file($uploadDir, $backupDir): array {
	#print_r2($_REQUEST);
	#print_r2(getallheaders());
	$res = false;
	if ($_SERVER['REQUEST_METHOD'] === 'POST') {
		// Check if the file was uploaded
		if (isset($_FILES['file']) && $_FILES['file']['error'] === UPLOAD_ERR_OK) {
			// Get file details
			$fileTmpPath = $_FILES['file']['tmp_name'];
			$fileName = $_FILES['file']['name'];
			if (!create_dir($uploadDir)) {
				$msg = get_return_code("create dir failed") . ": Failed to create upload dir $uploadDir";
			} else {
				$uploadFilePath = $uploadDir . '/' . basename($fileName);
				// Move the uploaded file to the upload directory
				if (move_uploaded_file($fileTmpPath, $uploadFilePath)) {
					chper($uploadFilePath);
					if ($backupDir)
						copy($uploadFilePath, $backupDir . '/' . basename($fileName));
					$msg = "0: File uploaded successfully: " . htmlspecialchars($fileName);
					$res = true;
				} else {
					$msg = get_return_code("file move failed") . ": Failed to move the uploaded file";
				}
			}
		} else {
			//echo $_FILES['file'];
			$msg = get_return_code("upload failed") . ": Error uploading file " . htmlspecialchars($fileName);
		}
	} else {
		$msg = get_return_code("invalid request method") . ": Invalid request method - " . $_SERVER['REQUEST_METHOD'];
	}
	return array($res, $msg);
}

function download_file($file, $downloadDir): array {
	$res = false;
	$msg = "";
	if (is_dir($downloadDir)) {
		echo ("isDir($downloadDir)");
		global $fileMode;
		ignore_user_abort(true);
		$files = array_values(array_diff(scandir($downloadDir), array('.', '..')));
		$ignore_extension = !str_contains($file, '.');
		$found = false;
		foreach ($files as &$existing_file) {
			if ($ignore_extension) {
				if ($file === substr($existing_file, 0, strlen($existing_file) - 4)) {
					$file = $existing_file;
					$found = true;
					break;
				}
			} elseif ($file === $existing_file) {
				$found = true;
				break;
			}
		}

		if ($found) {
			$filepath = $downloadDir . '/' . $existing_file;
			$filename = basename($filepath)."^^";
			$size = strlen($filename) + filesize($filepath);
			header('Content-Description: File Transfer');
			if (substr($filepath,-4) == ".txt" || substr($filepath,-4) == ".cmd" || substr($filepath,-4) == ".ini")
				header('Content-Type: text/plain');
			else
				header('Content-Type: application/octet-stream');
			header('Content-Disposition: attachment; filename="' . basename($filepath) . '"');
			header('Expires: 0');
			header('Cache-Control: must-revalidate');
			header('Pragma: public');
			header('Content-Length: ' . $size);
			$handle = fopen($filepath, 'rb');
			if ($handle) {
				while (!feof($handle) && !connection_aborted()) {
					echo fread($handle, 8192); // Read and output in chunks
					flush(); // Flush output buffer
				}
				fclose($handle);
				if (connection_aborted())
					$msg = get_return_code("connection aborted") . ": Connection aborted **download_file**";
				else {
					$res = true;
					$msg = ""; //only return the contents of the file. Any extra string will only get in the way
				}
			} else {
				$msg = get_return_code("open read failed") . ": failed to read $downloadDir/$file **download_file**";
			}
		} else {
			$msg = get_return_code("file not found") . ": $downloadDir/$file not found **download_file**";
		}
	} else {
		$msg = get_return_code("directory not found") . ": $downloadDir not found **download_file**";
	}
	return array($res, $msg);
}

function check_file_ctime($filename): array {
	print_r2($filename);
	if (is_file($filename)) {
		$res = true;
		$msg = "0: " . date('Hisymd', filectime($filename));
	} else {
		$res = false;
		$msg = get_return_code("file not found") . ": $filename not found **check_file_ctime**";
	}
	return array($res, $msg);
}

function scan_dir($path, $pattern, $incl_files, $incl_dirs, $get_c_time, $recursive, $subdir = ""): string {
	if (!str_ends_with($path, "/"))
		$path .= "/";
	$output = "";
	if (is_dir($path)) {
		$files = array_values(array_diff(scandir($path), array('.', '..')));
		if (count($files) > 0) {
			$use_pattern = strlen($pattern) > 0;
			foreach ($files as &$file) {
				$fullpath = $path.$file;
				$isdir = is_dir($fullpath);
				if (strlen($pattern) > 0) {
					if (!str_contains($file, $pattern)) {
						if (!$isdir && !$recursive)
							continue;
					}
				}
				if ($isdir) {
					if ($incl_dirs)
						$output .= $subdir !== "" ? "$subdir/$file|" : "$file|%|";
					if ($recursive)
						$output .= scan_dir($fullpath, $pattern, $incl_files, $incl_dirs, $get_c_time, $recursive, "$subdir/$file");
				} elseif (is_file($fullpath) && $incl_files) {
					$output .= $subdir !== "" ? "$subdir/$file|" : "$file|";
					if ($get_c_time)
						$output .= date('Hisymd', filectime($fullpath)) . "|";
				}
			}
		}
	}
	return $output;
}

function remove_from_string($bigstr, $smallstr): string {
	$start_pos = strpos($bigstr, $smallstr, 0);
	if ($start_pos >= 0) {
		$ret_str = substr($bigstr, 0, $start_pos);
		$ret_str .= substr($bigstr, $start_pos + strlen($smallstr), strlen($bigstr) - $start_pos - strlen($smallstr));
		return $ret_str;
	} else {
		return $bigstr;
	}
}

function run_commands($target_user, $subdir, $delete_cmdfile): array {
	global $rootdir;
	$path = $rootdir.$target_user.'/'.$subdir.'/';
	$res = false;
	if (is_dir($path)) {
		$files = array_values(array_diff(scandir($path), array('.', '..')));
		if (count($files) > 0) {
			global $scriptsdir;
			$script=$scriptsdir . "runcmds.sh";
			foreach ($files as &$file) {
				if (!str_ends_with($file, ".cmd"))
					continue;
				//echo "$script $target_user $subdir $file";
				$return_var = shell_exec("$script $target_user $subdir $file");
				if (is_null($return_var)) {
					$msg = get_return_code("unknown error") . ": an error occured or the command produced no output **run_commands**";
				} elseif (!$return_var) {
					$msg = get_return_code("unknown error") . ": the pipe could not be established **run_commands**";
				} else {
					if ($return_var == 0) {
						$res = true;
						$msg = "0: " . $file . " executed correctly";
					} else {
						$msg = get_return_code("exec failed") . ": $file($return_var)";
					}
				}
				if ($delete_cmdfile == "1")
					unlink($path.$file);
			}
			return true;
		} else {
			$msg = get_return_code("file not found") . ": $path is empty **run_commands**";
		}
	} else {
		$msg = get_return_code("directory not found") . ": $path does not exist **run_commands**";
	}
	return array($res, $msg);
}

function cmd_downloaded($target_user, $deviceid, $cmd_file): array {
	$cmds = file($cmd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	$new_cmds = "";
	$n_downloads = 1;
	$owner_download = false;
	$res = false;
	foreach ($cmds as $cmd) {
		if (str_contains($cmd, $deviceid)) {
			$owner_download = true;
			break;
		} else if (str_contains($cmd, "#Downloads")) {
			$n_downloads = substr($cmd, -1);
			$n_downloads++;
			$new_cmds .= "#Downloads $n_downloads";
		} else {
			$new_cmds .= $cmd;
		}
	}
	$n_devices = get_number_of_devices($target_user);
	if ($n_downloads >= $n_devices) {
		unlink($cmd_file);
	} else {
		if (!$owner_download) {
			$fh = fopen($cmd_file, "w");
			if ($fh) {
				chper($cmd_file);
				fwrite($fh, $new_cmds . "\n");
				fclose($fh);
				$res = true;
				$msg = "";
			} else {
				$msg = get_return_code("open write failed") . ": Unable to create or write to $cmd_file";
			}
		}
	}
	return array($res, $msg);
}

function set_online_visible($target_user, $visible): void {
	global $rootdir;
	$visible_file = "$rootdir$target_user/visible";
	if ($visible) {
		$fh = fopen($visible_file, "c");
		$ip = $_SERVER['REMOTE_ADDR'];
		fwrite($fh, "$ip\n");
		fclose($fh);
		/*$iter = new APCUIterator(null, APC_ITER_KEY);
		foreach ($iter as $key => $value) {
			echo $value['key'] . "\n";
		}*/
	} else {
		unlink($visible_file);
	}
}

function add_device($target_user, $device_id): array {
	global $fileMode;
	global $rootdir;
	$fh = null;
	$res = false;
	$devices_file = "$rootdir$target_user/devices.txt";
	if (!file_exists($devices_file)) {
		$fh = fopen($devices_file, "w");
		if ($fh)
			chper($devices_file);
		else
			$msg = get_return_code("open write failed") . ": Unable to create user's devices file $devices_file";
	} else {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $line) {
			if ($line == $device_id) {
				$res = true;
				$msg = get_return_code("no changes success") . ": Device already in the user's devices list";
				break;
			}
		}
		if ($res === false) {
			$fh = fopen($devices_file, "a+");
			if (!$fh)
				$msg = get_return_code("open write failed") . ": Unable to open user's devices file $devices_file";
		}
	}
	if ($fh && $res === false) {
		fwrite($fh, "$device_id\n");
		fclose($fh);
		$res = true;
		$msg = "0: Device added to the user's devices file.";
	}
	return array($res, $msg);
}

function del_device($target_user, $device_id): array {
	global $rootdir;
	$res = false;
	$devices_file = "$rootdir$target_user/devices.txt";
	if (file_exists($devices_file)) {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $line) {
			if ($line != $device_id)
				$new_devices = $new_devices . $line . "\r\n";
		}
		$fh = fopen($devices_file, "w");
		if ($fh) {
			fwrite($fh, $new_devices);
			fclose($fh);
			$res = true;
			$msg = "0: Device removed from the user's devices file";
		} else {
			$msg = get_return_code("open write failed") . ": Unable to open user's devices file! $devices_file";
		}
	} else {
		$msg = get_return_code("file not found") . ": User's devices file does not exist";
	}
	return array($res, $msg);
}

function get_number_of_devices($target_user): int {
	global $rootdir;
	$devices_file = "$rootdir$target_user/devices.txt";
	$n_devices = 0;
	if (file_exists($devices_file)) {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $device)
			$n_devices++;
	}
	return $n_devices;
}

function get_devices_list($target_user): array {
	global $rootdir;
	$devices_file = "$rootdir$target_user/devices.txt";
	if (file_exists($devices_file)) {
		$res = true;
		$msg = "0: ";
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $device) {
			$msg .= "$device|";
		}
	} else {
		$res = false;
		$msg = get_return_code("file not found") . ": User's device file does not exist";
	}
	return array($res, $msg);
}

function is_device_listed($target_user, $device_id): bool {
	global $rootdir;
	$devices_file = $rootdir . $target_user . "/devices.txt";
	if (file_exists($devices_file)) {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $device) {
			if ($device == $device_id)
				return true;
		}
	}
	return false;
}

function add_coach($coach): array {
	global $coaches_file;
	$res = false;
	if (!file_exists($coaches_file)) {
		$fh = fopen($coaches_file, "w");
		if (!$fh)
			$msg = get_return_code("open write failed") . ": Unable to create coaches file $coaches_file";
		else
			chper($coaches_file);
	} else {
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line == $coach) {
				$msg = get_return_code("no changes success") . ": Coach already in the public coaches list";
				$res = true;
				break;
			}
		}
		if ($res === false) {
			$fh = fopen($coaches_file, "a+");
			if (!$fh)
				$msg = get_return_code("open write failed") . ": Unable to open coaches file $coaches_file";
		}
	}
	if ($fh && $res === false) {
		fwrite($fh, $coach . "\n");
		fclose($fh);
		$msg = "0: Coach added to the public coaches list";
		$res = true;
	}
	return array($res, $msg);
}

function del_coach($coach): array {
	global $coaches_file;
	$res = false;
	if (file_exists($coaches_file)) {
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line != $coach)
				$new_coaches .= $line . "\r\n";
		}
		$fh = fopen($coaches_file, "w");
		if ($fh) {
			fwrite($fh, $new_coaches);
			fclose($fh);
			$res = true;
			$msg = "0: Coach removed from the public coaches list";
		} else {
			$msg = get_return_code("open write failed") . ": Unable to open coaches file $coaches_file";
		}
	} else {
		$msg = get_return_code("file not found") . ": Public coaches file does not exist";
	}
	return array($res, $msg);
}

function get_online_coaches(): array {
	global $coaches_file;
	if (file_exists($coaches_file)) {
		$res = true;
		$msg = "0: ";
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $coach) {
			$msg .= "$coach ";
		}
	} else {
		$res = false;
		$msg = get_return_code("file not found") . ": Public coaches file does not exist";
	}
	return array($res, $msg);
}

function request_coach($target_user, $coach): array {
	global $rootdir;
	$res = false;
	$requests_file = "$rootdir$coach/requests.txt";
	if (!file_exists($requests_file)) {
		$fh = fopen($requests_file, "w");
		if ($fh)
			chper($requests_file);
		else
			$msg = get_return_code("open create failed") . ": Unable to create requests file $requests_file";
	} else {
		$clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line == $target_user) {
				$res = true;
				$msg = get_return_code("no changes success") . ": Client's request had already been placed";
				break;
			}
		}
		if ($res === false) {
			$fh = fopen($requests_file, "a+");
			if (!$fh)
				$msg = get_return_code("open write failed") . ": Unable to open $requests_file to append new request";
		}
	}
	if ($fh && $res === false) {
		fwrite($fh, "$target_user\n");
		fclose($fh);
		$res = true;
		$msg = "0: Client's request to coach OK";
	}
	return array($res, $msg);
}

function delete_client_request($coach, $client): array {
	global $rootdir;
	$res = false;
	$requests_file = "$rootdir$coach/requests.txt";
	if (file_exists($requests_file)) {
		$clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line != $client)
				$new_clients .= "$line\n";
		}
		$fh = fopen($requests_file, "w");
		if ($fh) {
			fwrite($fh, $new_clients);
			fclose($fh);
			$res = true;
			$msg = "0: Client removed from the coach's request file.";
		} else {
			$msg = get_return_code("open write failed") . ": Unable to open requests file $requests_file";
		}
	} else {
		$msg = get_return_code("file not found") . ": Coach's $coach requests file does not exist";
	}
	return array($res, $msg);
}

function list_clients_requests($coach) {
	global $rootdir;
	$res = false;
	$requests_file = $rootdir . $coach . "/requests.txt";
	if (file_exists($requests_file)) {
		$res = true;
		$msg = "0: ";
		$clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $client) {
			$msg .= "$client ";
		}
	} else {
		$msg = get_return_code("file not found") . ": Coach $coach's requests file does not exist";
	}
	return array($res, $msg);
}

function accept_client_request($coach, $client): array {
	$result = delete_client_request($coach, $client);
	$res = $result[0];
	$msg = $result[1];
	if ($result[0]) {
		$res = false;
		global $rootdir;
		$accepts_file = "$rootdir$client/coaches_accepted.txt";
		if (!file_exists($accepts_file)) {
			$fh = fopen($accepts_file, "w");
			if ($fh)
				chper($accepts_file);
			else
				$msg = get_return_code("open write failed") . ": Unable to create client's accepts file $accepts_file";
		} else {
			$coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
			foreach ($coaches as $line) {
				if ($line == $coach) {
					$res = true;
					$msg = get_return_code("no changes success") . ": Coach's acceptance answer had already been placed.";
					break;
				}
			}
			if ($res === false) {
				$fh = fopen($accepts_file, "a+");
				if (!$fh)
					$msg = get_return_code("open write failed") . ": Unable to open accepts file to append new anwer $accepts_file";
			}
		}
		if ($fh && $res === false) {
			fwrite($fh, "$coach\n");
			fclose($fh);
			$res = true;
			$msg = "0: Coach $coach's answer to $client's request is OK";
		}
	}
	return array($res, $msg);
}

function reject_client_request($coach, $client) {
	$result = delete_client_request($coach, $client);
	$res = $result[0];
	$msg = $result[1];
	if ($result[0]) {
		$res = false;
		$rejects_file = "$rootdir$client/coaches_rejected.txt";
		if (!file_exists($rejects_file)) {
			$fh = fopen($rejects_file, "w");
			if ($fh)
				chper($rejects_file);
			else
				$msg = get_return_code("open write failed") . ": Unable to create client's rejections file $rejects_file";
		} else {
			$coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
			foreach ($coaches as $line) {
				if ($line === $coach) {
					$res = true;
					$msg = get_return_code("no changes success") . ": Coach's rejection answer had already been placed.";
					break;
				}
			}
			if ($res === false) {
				$fh = fopen($rejects_file, "a+");
				if (!$fh)
					$msg = get_return_code("open write failed") . ": Unable to open rejections file to append new answer $rejects_file";
			}
		}
		if ($fh && $res === false) {
			fwrite($fh, "$coach\n");
			fclose($fh);
			$res = true;
			$msg = "0: Coach's answer to client's request is REJECTED";
		}
	}
	return array($res, $msg);
}

function delete_coach_answer($client, $coach): array {
	$answers = "";
	global $rootdir;
	$accepts_file = "$rootdir$client/coaches_accepted.txt";
	if (file_exists($accepts_file)) {
		$coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line != $coach)
				$answers .= "$line\n";
		}
		$fh = fopen($accepts_file, "w");
		if ($fh) {
			fwrite($fh, $answers);
			fclose($fh);
			$res = true;
			$msg = "Coach's answer removed from the client's accepts file.";
		} else {
			$msg = "Unable to open client's accepts file $accepts_file.";
		}

		$rejects_file = "$rootdir$client/coaches_rejected.txt";
		if (file_exists($rejects_file)) {
			$answers = "";
			$coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
			foreach ($coaches as $line) {
				if ($line != $coach)
					$answers .= "$line\n";
			}
			$fh = fopen($rejects_file, "w");
			if ($fh) {
				fwrite($fh, $answers);
				fclose($fh);
				$res = true;
				$msg = "0: Coach's answer removed from the client's rejections file.";
			} else {
				$res = false;
				$msg = get_return_code("open write failed") . ": Unable to open client's rejections file $rejects_file" . $msg;
			}
		} else {
			if ($res === false)
				$msg = get_return_code("file not found") . ": Client's rejected coaches file does not exist" . $msg;
		}
	} else {
		$msg = get_return_code("file not found") . ": Client's accepted coaches file does not exist";
	}
	return false;
}

function list_coaches_answers($client): array {
	$answer = "";
	global $rootdir;

	$accepts_file = $rootdir . $client . "/coaches_accepted.txt";
	if (file_exists($accepts_file)) {
		$coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $coach)
			$answer .= "$coach.AOK ";
	}
	$rejects_file = $rootdir . $client . "/coaches_rejected.txt";
	if (file_exists($rejects_file)) {
		$coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $coach)
			$answer .= "$coach.NAY ";
	}
	if ($answer != "") {
		$res = true;
		$msg = "0: $answer";
	} else {
		$res = false;
		$msg = get_return_code("custom errors") . ": No coaches's answers";
	}
	return array($res, $msg);
}

function accept_coach_answer($client, $coach): array {
	ob_start();
	delete_coach_answer($client, $coach);
	ob_end_clean();
	global $rootdir;
	$res = false;
	$clients_file = "$rootdir$coach/clients.txt";
	if (!file_exists($clients_file)) {
		$fh = fopen($clients_file, "w");
		if ($fh)
			chper($clients_file);
		else
			$msg = get_return_code("open write failed") . ": Unable to create coach's clients file! $clients_file";
	} else {
		$clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line == $client) {
				$res = true; //get_return_code("no changes success") . ": $coach is already a coach of $client";
				break;
			}
		}
		if ($res === false) {
			$fh = fopen($clients_file, "a+");
			if (!$fh)
				$msg = get_return_code("open write failed") . ": Unable to open coach's clients file for appending! $clients_file";
		}
	}
	if ($fh && $res === false) {
		fwrite($fh, "$client\n");
		fclose($fh);
		$res = true;
	}
	if ($res === false)
		return array(false, $msg);
	else
		$res = false;

	$coaches_file = "$rootdir$client/coaches.txt";
	if (!file_exists($coaches_file)) {
		$fh = fopen($coaches_file, "w");
		if ($fh)
			chper($coaches_file);
		else
			$msg = get_return_code("open write failed") . ": Unable to create client's coaches file! $coaches_file";
	} else {
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line == $coach) {
				$res = true;
				$msg = get_return_code("no changes success") . ": $client. is already a client of $coach";
				break;
			}
		}
		if ($res === false) {
			$fh = fopen($coaches_file, "a+");
			if ($fh)
				$msg = get_return_code("open write failed") . ": Unable to open client's coaches file for appending! $coaches_file";
		}
	}
	if ($fh && $res === false) {
		fwrite($fh, "$coach\n");
		fclose($fh);
		$res = true;
		$msg = "0: " . $client . " is now a client of ".$coach;
	}
	return array($res, $msg);
}

function reject_coach_answer($client, $coach): array {
	return delete_coach_answer($client, $coach);
}

function get_clients_list($coach): array {
	global $rootdir;
	$clients_file = $rootdir . $coach . "/clients.txt";
	if (file_exists($clients_file)) {
		$res = true;
		$msg = "0: ";
		$clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $client)
			$msg .= "$client ";
	} else {
		$res = false;
		$msg = get_return_code("file not found") . ": Coach's clients file does not exist $clients_file";
	}
	return array($res, $msg);
}

function remove_client_from_clients($coach, $client): array {
	global $rootdir;
	$clients_file = "$rootdir$coach/clients.txt";

	if (file_exists($clients_file)) {
		$clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line != $client)
				$cur_clients .= "$line\n";
		}
		$fh = fopen($clients_file, "w");
		if ($fh) {
			fwrite($fh, $cur_clients);
			fclose($fh);
			$res = true;
			$msg = "0: Client removed from the coach's clients list";
		}
		else {
			$res = false;
			$msg = get_return_code("open write failed") . ": Unable to open coach's clients file! $clients_file";
		}
	}
	return array($res, $msg);
}

function get_coaches_list($client): array {
	global $rootdir;
	$coaches_file = "$rootdir$client/coaches.txt";
	if (file_exists($coaches_file)) {
		$res = true;
		$msg = "0: ";
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $coach)
			$msg .= "$coach ";
	} else {
		$res = false;
		echo get_return_code("file not found") . ": Coaches file does not exist" . $coaches_file;
	}
	return array($res, $msg);
}

function remove_coach_from_coaches($client, $coach): array {
	global $rootdir;
	$coaches_file = "$rootdir$client/coaches.txt";

	if (file_exists($coaches_file)) {
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line != $coach)
				$cur_coaches .= "$line\n";
		}
		$fh = fopen($coaches_file, "w");
		if ($fh) {
			fwrite($fh, $cur_coaches);
			fclose($fh);
			$res = true;
			$msg = "0: Coach removed from the client's coaches list";
		} else {
			$res = false;
			$msg = get_return_code("open write failed") . ": Unable to open client's coaches file $coaches_file";
		}
	}
	return array($res, $msg);
}

function get_tpmessages($owner_user): array {
	global $rootdir;
	$res = false;
	$tpmessages_file = $rootdir . $owner_user . "/tpmessages.txt";
	if (file_exists($tpmessages_file)) {
		$has_new_messages = apcu_fetch($owner_user."tpmessages");
		if ($has_new_messages === false) {
			$msg = get_return_code("no new messages") . ": No New TP messages";
		} else {
			$res = true;
			$msg = "0: ";
			$messages = file($tpmessages_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
			foreach ($messages as $message)
				$msg .= "$message\n";
			apcu_store($owner_user."tpmessages", false); //$has_new_messages = false
		}
	} else {
		$msg = get_return_code("no messages") . ": No TP messages";
	}
	return array($res, $msg);
}

function send_tpmessage($target_user, $message): array {
	global $rootdir;
	$res = false;
	$tpmessages_file = "$rootdir$target_user/tpmessages.txt";
	if (!file_exists($tpmessages_file)) {
		$fh = fopen($tpmessages_file, "w");
		if ($fh)
			chper($tpmessages_file);
		else
			 $msg = get_return_code("open write failed") . ": Unable to create TP messages file $tpmessages_file";
	} else {
		$existing_messages = file($tpmessages_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($existing_messages as $existing_message) {
			if ($existing_message == $message) {
				$res = true;
				$msg = get_return_code("no change success") . ": Message had already been sent";
				break;
			}
		}
		if ($res === false) {
			$fh = fopen($tpmessages_file, "a+");
			if (!$fh)
				$msg = get_return_code("open write failed") . ": Unable to append to to TP messages file $tpmessages_file";
		}
		if ($fh && $res === false) {
			fwrite($fh, "$message\n");
			fclose($fh);
			apcu_store($target_user."tpmessages", true); //$has_new_messages === true
			$res = true;
			$msg = "0: TP Message Sent!";
		}
	}
	return array($res, $msg);
}

function remove_tpmessage($target_user, $message): array {
	global $rootdir;
	$tpmessages_file = "$rootdir$target_user./tpmessages.txt";
	$tpmessages_arr = file($tpmessages_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	$kept_messages = [];
	$n_removed = 0;
	$n_kept = 0;
	foreach ($messages_arr as $stored_message) {
		if ($stored_message !== $message) {
			$kept_messages[] = $stored_message;
			$n_kept++;
		} else {
			$n_removed++;
		}
	}
	if ($n_removed > 0) {
		$fh = fopen($tpmessages_file, "w");
		if (count($kept_messages) > 0)
			fwrite($fh, implode($kept_messages));
		else
			fwrite($fh, "");
		fclose($fh);
		$res = true;
		$msg = "0: Message removed: $message";
	} else {
		$res = false;
		$msg = get_return_code("no changes success") . ": Cannot remove message because it was not found $message";
	}
	return array($res, $msg);
}

/* record_separator(oct 036, dec 30) separates the message fields
 * set_separator (oct 037, dec 31) separates messages of the same sender
 * exercises_separator (oct 034 dec 28) separates the senders
**/

function get_newchatmessages($owner_user): array {
	global $rootdir;
	$owner_user_dir = $rootdir . $owner_user;
	if (!create_dir($owner_user_dir))
		return array(false, get_return_code("directory not writable") . ": Unable to create dir: $owner_user_dir");

	$res = false;
	$files_str = scan_dir($owner_user_dir, "new-messages.chat", false, true, false, true);
	$files = explode('|', $files_str);
	if (count($files) > 0) {
		$content = "";
		foreach ($files as $file) {
			if ($file === "")
				continue;
			//apcu_store("$owner_user-$file", true); //uncomment to force new messages checking during development
			$is_modified = apcu_fetch("$owner_user-$file");
			if ($is_modified === false)
				continue;
			apcu_store("$owner_user-$file", false);
			$content .= file_get_contents($file);
		}
		if ($content != "") {
			$res = true;
			$msg = "0: $content";
		} else {
			$msg = get_return_code("no new messages") . ": No new chat messages";
		}
	} else {
		$msg = get_return_code("no messages") . ": No chat messages";
	}
	return array($res, $msg);
}

function send_chatmessage($sender, $receiver, $message): array {
	global $rootdir;
	$res = false;
	$messages_dir = "$rootdir$receiver/$sender/chat/";
	if (!create_dir($messages_dir)) {
		$msg = get_return_code("directory not writable") . ": Unable to create messages dir $messages_dir";
	} else {
		$messages_file = $messages_dir . "new-messages.chat";
		if (!file_exists($messages_file)) {
			$fh = fopen($messages_file, "w");
			if ($fh)
				chper($messages_file);
			else
				$msg = get_return_code("open write failed") . ": Unable to create messages file $messages_file";
		} else {
			$fh = fopen($messages_file, "a+");
			if (!$fh)
				$msg = get_return_code("open write failed") . ": Unable to append new message to $messages_file";
		}
		if ($fh) {
			fwrite($fh, "$message\037");
			fclose($fh);
			apcu_store("$receiver-$message_file", true); //set $is_modified = true
			$res = true;
			$msg = "0: Message Sent!";
		}
	}
	return array($res, $msg);
}

function remove_chat_message($sender, $recipient, $message): void {
	global $rootdir;
	$messages_file = "$rootdir$receiver/$sender/chat/new-messages.chat";
	$messages_str = file_get_contents($messages_file);
	$messages = explode("\037", $messages_str);
	$kept_messages = [];
	foreach ($messages as $_message) {
		if ($_message !== $message)
			$kept_messages[] = $message;
	}
	$fh = fopen($messages_file, "w");
	fwrite($fh, implode("\037", $kept_messages));
	fclose($fh);
}

function run_htpasswd($cmd_args, $target_user, $password): array {
	global $htpasswd;
	global $htpasswd_file;
	 if (!file_exists($htpasswd_file))
		$cmd_args = $cmd_args . "c";
	$return_var = 0;
	$output = [];
	exec("$htpasswd $cmd_args $htpasswd_file $target_user $password", $output, $return_var);
	return array($return_var, $output);
}

function user_exists($target_user, $user_password): array {
	$retur_var = run_htpasswd("-bv", $target_user, $user_password);
	switch ($return_var) {
	case 0:		$msg = "0: User exists and password is correct"; break;
	case 3:		$msg = get_return_code("wrong password") . ": User exists and password is wrong"; break;
	case 6:		$msg = get_return_code("user does not exist") . ": User does not exist"; break;
	default:	$msg = get_return_code("authentication failed") . ": Authentication Failed. Invalid user or password."; break;
	}
	return array($return_var === 0, $msg);
}

function update_datafile_with_password($target_user): bool {
	global $rootdir;
	global $htpasswd_file;

	$users = file($htpasswd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	$password="";
	foreach ($users as $user) {
		$sep=strpos($user, ':');
		$target_user=substr($user, 0, $sep);
		if ($target_user == $target_user) {
			$password=substr($user, $sep + 1, strlen($user) - $sep);
			break;
		}
	}
	if ($password != "") {
		$userdatafile = $rootdir . $target_user . "/user.data";
		if (file_exists($userdatafile)) {
			$fh = fopen($userdatafile, 'a');
			if ($fh) {
				fwrite($fh, $password);
				fclose($fh);
				return true;
			}
		}
	}
	return false;
}

function run_dbscript($cmd, $cmd_opt, $target_user): array {
	global $scriptsdir;
	$dbscript=$scriptsdir . "usersdb.sh";
	ob_start();
	if ($target_user) {
		if ($cmd_opt)
			passthru("$dbscript $target_user $cmd $cmd_opt", $return_var);
		else
			passthru("$dbscript $target_user $cmd", $return_var);
	} else {
		if ($cmd_opt)
			passthru("$dbscript $cmd $cmd_opt", $return_var);
		else
			passthru("$dbscript $cmd", $return_var);
	}
	$output = ob_get_clean();
	return array($return_var, $output);
}

?>
