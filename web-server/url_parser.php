<?php

$rootdir="/var/www/html/trainingplanner/";
$pause_file=$rootdir . "pause";
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
	$dirMode = 0755;
	$fileMode = 0644;
}

// set the default timezone to use.
date_default_timezone_set('America/Sao_Paulo');

function print_r2($val){
		echo '<pre>';
		print_r($val);
		echo  '</pre>';
}

function get_return_code($desc) {
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
	return 100; //Unknown error code
}

function erasedir($path) {
	$dir = opendir($path);
	while( false !== ($file = readdir($dir)) ) {
		if (( $file != '.' ) && ( $file != '..' )) {
			$full = $path . '/' . $file;
			if (is_dir($full))
				erasedir($full);
			else if (is_file($full))
				unlink($full);
		}
	}
	closedir($dir);
	return rmdir($path);
}

//!!Attention!! mkdir() does not set the permissions specified. Must use chmod() afterwards
function create_dir($directory) {
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
function verify_credentials($username, $password, $htpasswd_file) {
	if (!file_exists($htpasswd_file)) {
		die("htpasswd file not found");
	}
	#print_r2("Authenticating " . $username . " with password " . $password);
	// Read the .htpasswd file line by line
	$lines = file($htpasswd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	foreach ($lines as $line) {
		// Each line is in format: username:hashed_password
		list($storedUser, $storedHash) = explode(':', $line, 2);

		if ($storedUser === $username) {
			// Check password against stored hash
			if (password_verify($password, $storedHash) || crypt($password, $storedHash) === $storedHash) {
				return true;
			}
		}
	}
	return false;
}

function upload_file($uploadDir) {
	#print_r2($_REQUEST);
	#print_r2(getallheaders());
	if ($_SERVER['REQUEST_METHOD'] === 'POST') {
		// Check if the file was uploaded
		if (isset($_FILES['file']) && $_FILES['file']['error'] === UPLOAD_ERR_OK) {
			// Get file details
			$fileTmpPath = $_FILES['file']['tmp_name'];
			$fileName = $_FILES['file']['name'];
			if (!create_dir($uploadDir)) {
				echo get_return_code("create dir failed") . ": Failed to create upload dir " . $uploadDir;
				return false;
			}
			$uploadFilePath = $uploadDir . "/" . basename($fileName);
			// Move the uploaded file to the upload directory
			if (move_uploaded_file($fileTmpPath, $uploadFilePath)) {
				chper($uploadFilePath);
				echo "0: File uploaded successfully: " . htmlspecialchars($fileName);
				return true;
			}
			else
				echo get_return_code("file move failed") . ": Failed to move the uploaded file";
		}
		else {
			//echo $_FILES['file'];
			echo get_return_code("upload failed") . ": Error uploading file " . htmlspecialchars($fileName);
		}
	}
	else
		echo get_return_code("invalid request method") . ": Invalid request method";
	return false;
}

function download_file($file, $downloadDir) {
	global $fileMode;
	ignore_user_abort(true);
	$filename=$downloadDir . "/" . $file;
	if (file_exists($filename)) {
		$file_name = basename($filename)."%%";
		$size = strlen($file_name) + filesize($filename);
		echo $file_name;
		header('Content-Description: File Transfer');
		if (substr($filename,-4) == ".txt" || substr($filename,-4) == ".cmd" || substr($filename,-4) == ".ini")
			header('Content-Type: text/plain');
		else
			header('Content-Type: application/octet-stream');
		header('Content-Disposition: attachment; filename="' . basename($filename) . '"');
		header('Expires: 0');
		header('Cache-Control: must-revalidate');
		header('Pragma: public');
		header('Content-Length: ' . $size);
		//readfile($filename);
		$handle = fopen($filename, 'rb');
		if ($handle) {
			while (!feof($handle) && !connection_aborted()) {
				echo fread($handle, 8192); // Read and output in chunks
				flush(); // Flush output buffer
			}
			fclose($handle);
			if (connection_aborted()) {
				echo get_return_code("connection aborted") . ": Connection aborted **download_file**";
				return false;
			}
			//only return the contents of the file. Any extra string will only get in the way
			return true;
		}
	}
	echo get_return_code("file not found") . ": ", $filename . " not found **download_file**";
	return false;
}

function get_binfile($binfile, $downloadDir) {
	if (is_dir($downloadDir)) {
		$files = array_values(array_diff(scandir($downloadDir), array('.', '..')));
		$ignore_extension = !str_contains($binfile, '.');
		foreach ($files as &$file) {
			$filename = basename($file);
			if ($ignore_extension) {
				if ($binfile != substr($filename, 0, strlen($filename) - 4))
					continue;
			}
			else {
				if ($binfile != $filename)
					continue;
			}
			download_file($filename, $downloadDir);
			return true;
		}
		echo get_return_code("file not found") . ": " . $binfile . " not found in " . $downloadDir, "**get_binfile**";
	}
	else
		echo get_return_code("directory not found") . ": ",  $downloadDir . " not found **get_binfile**";
}

function check_file_ctime($filename) {
	if (is_file($filename))
		echo "0 :", date('Hisymd', filectime($filename));
	else
		echo get_return_code("file not found") . ": " . $filename . " not found **check_file_ctime**";
}

function scan_dir($path, $pattern, $only_dirs, $only_files, $get_c_time) {
	$output = "";
	if (is_dir($path)) {
		$files = array_values(array_diff(scandir($path), array('.', '..')));
		if (count($files) > 0) {
			echo "0: ";
			foreach ($files as &$file) {
				if (strlen($pattern) > 0) {
					if (!str_contains($file, $pattern))
						continue;
				}
				if ($only_dirs && !is_dir($path . $file))
					continue;
				if ($only_files && !is_file($path . $file))
					continue;
				if ($get_c_time)
					$output .= $file . "|" . date('Hisymd', filectime($path.$file)) . "|";
				else
					$output .= $file . "|";
			}
		}
		else
			$output = get_return_code("directory empty") . ": " . $path . " is empty **scan_dir**";
	}
	else
		$output = get_return_code("directory not found") . ": " . $path . " does not exist **scan_dir**";
	return $output;
}

function remove_from_string($bigstr, $smallstr) {
	$start_pos = strpos($bigstr, $smallstr, 0);
	if ($start_pos >= 0) {
		$ret_str = substr($bigstr, 0, $start_pos);
		$ret_str .= substr($bigstr, $start_pos + strlen($smallstr), strlen($bigstr) - $start_pos - strlen($smallstr));
		return $ret_str;
	}
	else
		return $bigstr;
}

function run_commands($userid, $subdir, $delete_cmdfile) {
	global $rootdir;
	$path = $rootdir.$userid.'/'.$subdir.'/';
	if (is_dir($path)) {
		$files = array_values(array_diff(scandir($path), array('.', '..')));
		if (count($files) > 0) {
			global $scriptsdir;
			$script=$scriptsdir . "runcmds.sh";
			//ob_start();
			foreach ($files as &$file) {
				if (!str_ends_with($file, ".cmd"))
					continue;
				//passthru("$script $userid $subdir $file", $return_var);
				//echo "$script $userid $subdir $file";
				$return_var = shell_exec("$script $userid $subdir $file");
				if (is_null($return_var))
					echo "an error occured or the command produced no output";
				elseif (!$return_var)
					echo "the pipe could not be established";
				else {
					if ($return_var == 0) {
						echo "0: " . $file . " executed correctly";
						if ($delete_cmdfile == "1")
							unlink($path.$file);
					}
					else
						echo get_return_code("exec failed") . ": " . $file . " (".$return_var.")";
				}
			}
			//$output = ob_get_clean();
			return true;
		}
	}
	echo get_return_code("directory not found") . ": " . $path . " is empty or does not exist **run_commands**";
	return false;
}

function cmd_downloaded($userid, $deviceid, $cmd_file) {
	$cmds = file($cmd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	$new_cmds="";
	$n_downloads=1;
	$owner_download = false;
	foreach ($cmds as $cmd) {
		if (str_contains($cmd, $deviceid)) {
			$owner_download = true;
			break;
		}
		else if (str_contains($cmd, "#Downloads")) {
			$n_downloads = substr($cmd, -1);
			++$n_downloads;
			$new_cmds = $new_cmds . "#Downloads " . $n_downloads . "";
		}
		else
			$new_cmds = $new_cmds . $cmd . "";
	}
	$n_devices = get_number_of_devices($userid);

	if ($n_downloads >= $n_devices)
		unlink($cmd_file);
	else {
		if (!$owner_download) {
			$fh = fopen($cmd_file, "w") or die(get_return_code("open write failed") . ": Unable to create or write to " . $cmd_file);
			chper($cmd_file);
			fwrite($fh, $new_cmds . "\n");
			fclose($fh);
		}
	}
}

function run_test_function($username, $password) {
	echo get_return_code("no changes success") . ": Some message";
	echo get_return_code("custom error") . ": Some message";
	echo get_return_code("directory not found") . ": Some message";
}

function add_device($userid, $device_id) {
	global $fileMode;
	global $rootdir;
	$devices_file = $rootdir . $userid . "/devices.txt";
	if (!file_exists($devices_file)) {
		$fh = fopen($devices_file, "w") or die(get_return_code("open write failed") . ": Unable to create user's devices file " . $devices_file);
		chper($devices_file);
	}
	else {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $line) {
			if ($line == $device_id) {
				echo get_return_code("no changes success") . ": Device already in the user's devices list";
				return;
			}
		}
		$fh = fopen($devices_file, "a+") or die(get_return_code("open write failed") . ": Unable to open user's devices file " .$devices_file);
	}
	fwrite($fh, $device_id . "\n");
	fclose($fh);
	echo "0: Device added to the user's devices file.";
}

function del_device($userid, $device_id) {
	global $rootdir;
	$devices_file = $rootdir . $userid . "/devices.txt";
	if (file_exists($devices_file)) {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $line) {
			if ($line != $device_id)
				$new_devices = $new_devices . $line . "\r\n";
		}
		$fh = fopen($devices_file, "w") or die(get_return_code("open write failed") . ": Unable to open user's devices file!" .$devices_file);
		fwrite($fh, $new_devices);
		fclose($fh);
		echo "0: Device removed from the user's devices file";
	}
	else
		echo get_return_code("file not found") . ": User's devices file does not exist";
}

function get_number_of_devices($userid) {
	global $rootdir;
	$devices_file = $rootdir . $userid . "/devices.txt";
	$n_devices = 0;
	if (file_exists($devices_file)) {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $device)
			$n_devices++;
	}
	return $n_devices;
}

function get_devices_list($userid) {
	global $rootdir;
	$devices_file = $rootdir . $userid . "/devices.txt";
	if (file_exists($devices_file)) {
		echo "0: ";
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $device) {
			echo $device . "|";
		}
	}
	else
		echo get_return_code("file not found") . ": User's device file does not exist";
}

function is_device_listed($userid, $device_id) {
	global $rootdir;
	$devices_file = $rootdir . $userid . "/devices.txt";
	if (file_exists($devices_file)) {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $device) {
			if ($device == $device_id)
				return true;
		}
	}
	return false;
}

function add_coach($coach) {
	global $coaches_file;
	if (!file_exists($coaches_file)) {
		$fh = fopen($coaches_file, "w") or die(get_return_code("open write failed") . ": Unable to create coaches file " . $coaches_file);
		chper($coaches_file);
	}
	else {
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line == $coach) {
				echo get_return_code("no changes success") . ": Coach already in the public coaches list";
				return;
			}
		}
		$fh = fopen($coaches_file, "a+") or die(get_return_code("open write failed") . ": Unable to open coaches file " . $coaches_file);
	}
	fwrite($fh, $coach . "\n");
	fclose($fh);
	echo "0: Coach added to the public coaches list";
}

function del_coach($coach) {
	global $coaches_file;
	if (file_exists($coaches_file)) {
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line != $coach)
				$new_coaches = $new_coaches . $line . "\r\n";
		}
		$fh = fopen($coaches_file, "w") or die(get_return_code("open write failed") . ": Unable to open coaches file " .$coaches_file);
		fwrite($fh, $new_coaches);
		fclose($fh);
		echo "0: Coach removed from the public coaches list";
	}
	else
		echo get_return_code("file not found") . ": Public coaches file does not exist";
}

function get_online_coaches() {
	global $coaches_file;
	if (file_exists($coaches_file)) {
		echo "0: ";
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $coach) {
			echo $coach . " ";
		}
	}
	else
		echo get_return_code("file not found") . ": Public coaches file does not exist";
}

function request_coach($username, $coach) {
	global $rootdir;
	$requests_file = $rootdir . $coach . "/requests.txt";
	if (!file_exists($requests_file)) {
		$fh = fopen($requests_file, "w") or die("Return code: 10 Unable to create requests file!" .$requests_file . "");
		chper($requests_file);
	}
	else {
		$clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line == $username) {
				echo get_return_code("no changes success") . ": Client's request had already been placed";
				return;
			}
		}
		$fh = fopen($requests_file, "a+") or die(get_return_code("open write failed") . ": Unable to open requests file to append new request " .$requests_file);
	}
	fwrite($fh, $username . "\n");
	fclose($fh);
	echo "0: Client's request to coach OK";
}

function delete_client_request($coach, $client) {
	global $rootdir;
	$requests_file = $rootdir . $coach . "/requests.txt";
	if (file_exists($requests_file)) {
		$clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line != $client) {
				$new_clients = $new_clients . $line . "\r\n";
			}
		}
		$fh = fopen($requests_file, "w") or die(get_return_code("open write failed") . ": Unable to open requests file " .$requests_file);
		fwrite($fh, $new_clients);
		fclose($fh);
		echo "0: Client removed from the coach's request file.";
		return true;
	}
	else {
		echo get_return_code("file not found") . ": Coach's " . $coach . " requests file does not exist";
		return false;
	}
}

function list_clients_requests($coach) {
	global $rootdir;
	$requests_file = $rootdir . $coach . "/requests.txt";
	if (file_exists($requests_file)) {
		echo "0: ";
		$clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $client) {
			echo $client . " ";
		}
	}
	else
		echo get_return_code("file not found") . ": Coach's " . $coach . " requests file does not exist";
}

function accept_client_request($coach, $client)
{
	if (delete_client_request($coach, $client)) {
		global $rootdir;
		$accepts_file = $rootdir . $client . "/coaches_accepted.txt";
		if (!file_exists($accepts_file)) {
			$fh = fopen($accepts_file, "w") or die(get_return_code("open write failed") . ": Unable to create client's accepts file " .$accepts_file);
			chper($accepts_file);
		}
		else {
			$coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
			foreach ($coaches as $line) {
				if ($line == $coach) {
					echo get_return_code("no changes success") . ": Coach's acceptance answer had already been placed.";
					return;
				}
			}
			$fh = fopen($accepts_file, "a+") or die(get_return_code("open write failed") . ": Unable to open accepts file to append new anwer " .$accepts_file);
		}
		fwrite($fh, $coach . "\n");
		fclose($fh);
		echo "0: Coach's(".$coach.") answer to client's(".$client.") request OK";
		return true;
	}
	echo get_return_code("unknown error") . ": Could not place coach's answer to client";
	return false;
}

function reject_client_request($coach, $client)
{
	if (delete_client_request($coach, $client)) {
		global $rootdir;
		$rejects_file = $rootdir . $client . "/coaches_rejected.txt";
		if (!file_exists($rejects_file)) {
			$fh = fopen($rejects_file, "w") or die(get_return_code("open write failed") . ": Unable to create client's rejections file " .$rejects_file);
			chper($rejects_file);
		}
		else {
			$coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
			foreach ($coaches as $line) {
				if ($line == $coach) {
					echo get_return_code("no changes success") . ": Coach's rejection answer had already been placed.";
					return;
				}
			}
			$fh = fopen($rejects_file, "a+") or die(get_return_code("open write failed") . ": Unable to open rejections file to append new answer " .$rejects_file);
		}
		fwrite($fh, $coach . "\n");
		fclose($fh);
		echo "0: Coach's answer to client's request is REJECTED";
		return true;
	}
	echo get_return_code("unknown error") . ": Could not place coach's rejection to client";
	return false;
}

function delete_coach_answer($client, $coach) {
	$answers = "";
	global $rootdir;
	$accepts_file = $rootdir . $client . "/coaches_accepted.txt";
	if (file_exists($accepts_file)) {
		$coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line != $coach)
				$answers = $answers . $line . "\r\n";
		}
		$fh = fopen($accepts_file, "w") or die(get_return_code("open write failed") . ": Unable to open client's accepts file " .$accepts_file);
		fwrite($fh, $answers);
		fclose($fh);

		$rejects_file = $rootdir . $client . "/coaches_rejected.txt";
		if (file_exists($rejects_file)) {
			$answers = "";
			$coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
			foreach ($coaches as $line) {
				if ($line != $coach)
					$answers = $answers . $line . "\r\n";
			}
			echo "0: Coache's answer removed from the client's rejections file.";
			return 0;
		}
	}
	echo get_return_code("file not found") . ": Client's accepted coaches file does not exist";
	return false;
}

function list_coaches_answers($client) {
	$answer = "";
	global $rootdir;

	$accepts_file = $rootdir . $client . "/coaches_accepted.txt";
	if (file_exists($accepts_file)) {
		$coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $coach) {
			$answer = $answer.$coach."AOK ";
		}
	}

	$rejects_file = $rootdir . $client . "/coaches_rejected.txt";
	if (file_exists($rejects_file)) {
		$coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $coach) {
			$answer = $answer.$coach."NAY ";
		}
	}
	if ($answer != "")
		echo "0: ".$answer;
	else
		echo get_return_code("custom errors") . ": No coaches's answers";
}

function accept_coach_answer($client, $coach)
{
	ob_start();
	delete_coach_answer($client, $coach);
	ob_end_clean();
	global $rootdir;
	$clients_file = $rootdir . $coach . "/clients.txt";
	if (!file_exists($clients_file)) {
		$fh = fopen($clients_file, "w")  or die(get_return_code("open write failed") . ": Unable to create coach's clients file! " . $clients_file);
		chper($clients_file);
	}
	else {
		$clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line == $client) {
				echo get_return_code("no changes success") . ": " . $client . " is already a client of " . $coach;
				return;
			}
		}
		$fh = fopen($clients_file, "a+")  or die(get_return_code("open write failed") . ": Unable to open coach's clients file for appending! " . $clients_file);
	}
	fwrite($fh, $client . "\n");
	fclose($fh);

	$coaches_file = $rootdir . $client . "/coaches.txt";
	if (!file_exists($coaches_file)) {
		$fh = fopen($coaches_file, "w")  or die(get_return_code("open write failed") . ": Unable to create client's coaches file! " . $coaches_file);
		chper($coaches_file);
	}
	else {
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line == $coach) {
				echo get_return_code("no changes success") . ": " . $client. " is already a client of " . $coach;
				return;
			}
		}
		$fh = fopen($coaches_file, "a+")  or die(get_return_code("open write failed") . ": Unable to open client's coaches file for appending! " . $coaches_file);
	}
	fwrite($fh, $coach . "\n");
	fclose($fh);
	echo "0: " . $client . " is now a client of ".$coach;
	return true;
}

function reject_coach_answer($client, $coach)
{
	return delete_coach_answer($client, $coach);
}

function get_clients_list($coach) {
	global $rootdir;
	$clients_file = $rootdir . $coach . "/clients.txt";
	if (file_exists($clients_file)) {
		echo "0: ";
		$clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $client) {
			echo $client . " ";
		}
	}
	else
		echo get_return_code("file not found") . ": Coach's clients file does not exist" . $clients_file;
}

function remove_client_from_clients($coach, $client) {
	global $rootdir;
	$clients_file = $rootdir . $coach . "/clients.txt";

	if (file_exists($clients_file)) {
		$clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line != $client) {
				$cur_clients = $cur_clients . $line . "\r\n";
			}
		}
		$fh = fopen($clients_file, "w") or die(get_return_code("open write failed") . ": Unable to open coach's clients file!" . $clients_file);
		fwrite($fh, $cur_clients);
		fclose($fh);
		echo "0: Client removed from the coach's clients list";
		return true;
	}
}

function get_coaches_list($client) {
	global $rootdir;
	$coaches_file = $rootdir . $client . "/coaches.txt";
	if (file_exists($coaches_file)) {
		echo "0: ";
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $coach) {
			echo $coach . " ";
		}
	}
	else
		echo get_return_code("file not found") . ": Coaches file does not exist" . $coaches_file;
}

function remove_coach_from_coaches($client, $coach) {
	global $rootdir;
	$coaches_file = $rootdir . $client . "/coaches.txt";

	if (file_exists($coaches_file)) {
		$coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($coaches as $line) {
			if ($line != $coach) {
				$cur_coaches = $cur_coaches . $line . "\r\n";
			}
		}
		$fh = fopen($coaches_file, "w") or die(get_return_code("open write failed") . ": Unable to open client's coaches file " .$coaches_file);
		fwrite($fh, $cur_coaches);
		fclose($fh);
		echo "0: Coach removed from the client's coaches list";
		return true;
	}
}

/*
	record_separator(oct 036, dec 30) separates the message fields
	set_separator (oct 037, dec 31) separates messages of the same sender
	exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and the odd numbers are the sender file names)
*/

function get_newmessages($username) {
	global $rootdir;
	$messages_dir = $rootdir . $username . "/chats";
	if (!create_dir($messages_dir))
		die(get_return_code("directory not writable") . ": Unable to create messages dir " .$messages_dir);

	$files = array_values(array_diff(scandir($messages_dir), array('.', '..')));
	if (count($files) > 0) {
		$content = "";
		foreach ($files as $file) {
			if (str_contains($file, ".sqlite"))
				continue;
			$content = $content . file_get_contents($messages_dir.'/'.$file);
			$content = $content . "\034" . $file . "\034";
		}
		if ($content != "")
		{
			echo "0: ";
			echo $content;
			return;
		}
	}
	echo get_return_code("directory empty") . ": No new messages";
}

function send_message($username, $receiver, $message) {
	global $rootdir;
	$messages_dir = $rootdir . $receiver . "/chats/";
	if (!create_dir($messages_dir))
		die(get_return_code("directory not writable") . ": Unable to create messages dir " .$messages_dir);
	$messages_file = $messages_dir . $username . ".msg";

	if (!file_exists($messages_file)) {
		$fh = fopen($messages_file, "w") or die(get_return_code("open write failed") . ": Unable to create messages file " . $messages_file);
		chper($messages_file);
	}
	else
		$fh = fopen($messages_file, "a+") or die(get_return_code("open write failed") . ": Unable to append to to messages file " . $messages_file);
	fwrite($fh, $message . "\037");
	fclose($fh);
	echo "0: Message Sent!";
}

function message_worker($sender, $recipient, $messageid, $argument) {
	global $rootdir;
	$messages_dir = $rootdir . $recipient . "/chats/";
	if (!create_dir($messages_dir))
		die(get_return_code("directory not writable") . ": Unable to create messages dir " .$messages_dir);
	$messages_file = $messages_dir . $sender . "." . $argument;
	if (!file_exists($messages_file)) {
		$fh = fopen($messages_file, "w") or die(get_return_code("open write failed") . ": Unable to create " . $messages_file);
		chper($messages_file);
	}
	else {
		$msgids = file($messages_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		if (str_contains($msgids, $messageid . "\037")) {
			echo get_return_code("no changes success") . ": Message " . $argument . "!";
			return;
		}
		$fh = fopen($messages_file, "a+") or die(get_return_code("open write failed") . ": Unable to append to " . $messages_file);
	}
	fwrite($fh, $messageid);
	fclose($fh);
	echo "0: Message " . $argument . "!";
}

function run_htpasswd($cmd_args, $username, $password) {
	global $htpasswd;
	global $htpasswd_file;

	 if (!file_exists($htpasswd_file))
		$cmd_args = $cmd_args . "c";
	$return_var = 0;
	$output = [];
	exec("$htpasswd $cmd_args $htpasswd_file $username $password", $output, $return_var);
	return $return_var;
}

function user_exists($username, $user_password) {
	return run_htpasswd("-bv", $username, $user_password);
}

function user_exists_return_message($return_var) {
	switch ($return_var) {
		case 0: $error_string = "0: User exists and password is correct"; break;
		case 3: $error_string = get_return_code("wrong password") . ": User exists and password is wrong"; break;
		case 6: $error_string = get_return_code("user does not exist") . ": User does not exist"; break;
		default: $error_string = get_return_code("user does not exist") . ": User does not exist"; break;
	}
	return $error_string;
}

function update_datafile_with_password($userid) {
	global $rootdir;
	global $htpasswd_file;

	$users = file($htpasswd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	$password="";
	foreach ($users as $user) {
		$sep=strpos($user, ':');
		$username=substr($user, 0, $sep);
		if ($username == $userid) {
			$password=substr($user, $sep+1, strlen($user)-$sep);
			break;
		}
	}
	if ($password != "") {
		$userdatafile = $rootdir . $userid . "/user.data";
		if (file_exists($userdatafile)) {
			$fh = fopen($userdatafile, 'a')  or die(get_return_code("open write failed") . ": Unable to open " .$userdatafile);
			fwrite($fh, $password);
			fclose($fh);
			return true;
		}
	}
	return false;
}

function run_dbscript($cmd, $cmd_opt, $userid, $print_output) {
	global $scriptsdir;
	$dbscript=$scriptsdir . "usersdb.sh";
	ob_start();
	if ($userid) {
		if ($cmd_opt)
			passthru("$dbscript $userid $cmd $cmd_opt", $return_var);
		else
			passthru("$dbscript $userid $cmd", $return_var);
	}
	else {
		if ($cmd_opt)
			passthru("$dbscript $cmd $cmd_opt", $return_var);
		else
			passthru("$dbscript $cmd", $return_var);
	}
	$output = ob_get_clean();
	if ($print_output)
		echo $output;
	return $return_var;
}

if (file_exists($pause_file)) {
	$paused = file($pause_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	if ($paused[0] == "1") {
		echo get_return_code("server paused") . ": TrainingPlanner app server paused!";
		exit;
	}
}

$username = isset($_GET['user']) ? $_GET['user'] : '';

if ($username) {
	$password = isset($_GET['password']) ? $_GET['password'] : '';
	if (!$password)
		die(get_return_code("password missing") . ": Missing password");
	else {
		if (verify_credentials($username, $password, $htpasswd_file)) {
			// Authentication successful

			if (isset($_GET['test'])) {
				run_test_function($username, $password);
				exit;
			}

			if (isset($_GET['listfiles'])) {
				$subdir = isset($_GET['listfiles']) ? $_GET['listfiles'] . "/" : '';
				$targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] ."/" : $username;
				if ($targetuser == "")
					$targetuser = $username;
				$filedir = $rootdir . $targetuser . $subdir;
				$pattern = isset($_GET['pattern']) ? $_GET['pattern'] : '';
				$files = scan_dir($filedir, $pattern, false, true, true);
				echo $files;
				exit;
			}
			if (isset($_GET['listdirs'])) {
				$subdir = isset($_GET['listdirs']) ? $_GET['listdirs'] . "/" : '';
				$targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] ."/" : $username;
				if ($targetuser == "")
					$targetuser = $username;
				$filedir = $rootdir . $targetuser . $subdir;
				$pattern = isset($_GET['pattern']) ? $_GET['pattern'] : '';
				$files = scan_dir($filedir, $pattern, true, false, false);
				echo $files;
				exit;
			}

			if (isset($_GET['runcmds'])) {
				$subdir = $_GET['runcmds'];
				$delete_cmdfile = isset($_GET['delete']) ? $_GET['delete'] : 0;
				run_commands($username, $subdir, $delete_cmdfile);
				exit;
			}

			if (isset($_GET['downloadcmd'])) {
				$subdir = isset($_GET['subdir']) ? $_GET['subdir'] . "/" : '';
				$filedir = $rootdir . $username . "/" . $subdir;
				$device_id = $_GET['deviceid'];
				if ($device_id == "")
					die(get_return_code("no device id") . ": Missing device id argument");
				else if (!is_device_listed($username, $device_id))
					die(get_return_code("deviceid not registered") . ": Device id not registered");
				if (download_file($_GET['downloadcmd'], $filedir))
					cmd_downloaded($username, $device_id, $filedir . $_GET['downloadcmd']);
				exit;
			}

			if ($username != "admin") {

				if (isset($_GET['adddevice'])) {
					$device_id = $_GET['adddevice'];
					add_device($username, $device_id);
					exit;
				}
				if (isset($_GET['deldevice'])) {
					$device_id = $_GET['deldevice'];
					del_device($username, $device_id);
					exit;
				}
				if (isset($_GET['getdeviceslist'])) {
					get_devices_list($username);
					exit;
				}

				if (isset($_GET['addcoach'])) {
					add_coach($username);
					exit;
				}
				if (isset($_GET['delcoach'])) {
					del_coach($username);
					exit;
				}

				if (isset($_GET['getonlinecoaches'])) {
					get_online_coaches();
					exit;
				}
				if (isset($_GET['requestcoach'])) {
					$coach = $_GET['requestcoach'];
					if ($coach)
						request_coach($username, $coach);
					exit;
				}
				if (isset($_GET['deleteclientrequest'])) {
					$client = $_GET['deleteclientrequest'];
					if ($client)
						delete_client_request($username, $client);
					exit;
				}
				if (isset($_GET['listclientsrequests'])) {
					list_clients_requests($username);
					exit;
				}
				if (isset($_GET['acceptclientrequest'])) {
					$client = $_GET['acceptclientrequest'];
					if ($client)
						accept_client_request($username, $client);
					exit;
				}
				if (isset($_GET['rejectclientrequest'])) {
					$client = $_GET['rejecttclientrequest'];
					if ($client)
						reject_client_request($username, $client);
					exit;
				}

				if (isset($_GET['deletecoachanswer'])) {
					$coach = $_GET['deletecoachanswer'];
					if ($coach)
						delete_coach_answer($username, $coach);
					exit;
				}
				if (isset($_GET['listcoachesanswers'])) {
					list_coaches_answers($username);
					exit;
				}
				if (isset($_GET['acceptcoachanswer'])) {
					$coach = $_GET['acceptcoachanswer'];
					if ($coach)
						accept_coach_answer($username, $coach);
					exit;
				}
				if (isset($_GET['rejectcoachanswer'])) {
					$coach = $_GET['rejectcoachanswer'];
					if ($coach)
						reject_coach_answer($username, $coach);
					exit;
				}

				if (isset($_GET['getclients'])) {
					get_clients_list($username);
					exit;
				}
				if (isset($_GET['removecurclient'])) {
					$client = $_GET['removecurclient'];
					if ($client)
						remove_client_from_clients($username, $client);
					exit;
				}

				if (isset($_GET['getcoaches'])) {
					get_coaches_list($username);
					exit;
				}
				if (isset($_GET['removecurcoach'])) {
					$coach = $_GET['removecurcoach'];
					if ($coach)
						remove_coach_from_coaches($username, $coach);
					exit;
				}

				if (isset($_GET['getnewmessages'])) {
					get_newmessages($username);
					exit;
				}
				if (isset($_GET['sendmessage'])) {
					$receiver=$_GET['sendmessage'];
					$receiver != "" or die(get_return_code("argument missing") . ": No receiver argument **sendmessage**");
					$message = $_GET['message'];
					$message != "" or die(get_return_code("argument missing") . ": No message argument **sendmessage**");
					send_message($username, $receiver, $message);
					exit;
				}
				if (isset($_GET['messagereceived'])) {
					$recipient=$_GET['messagereceived'];
					$recipient != "" or die(get_return_code("argument missing") . ": No sender argument **messagereceived**");
					$messageid = $_GET['messageid'];
					$messageid != "" or die(get_return_code("argument missing") . ": No message id argument **messagereceived**");
					message_worker($username, $recipient, $messageid, "received");
					exit;
				}
				if (isset($_GET['messageread'])) {
					$recipient=$_GET['messageread'];
					$recipient != "" or die(get_return_code("argument missing") . ": No sender argument **messageread**");
					$messageid = $_GET['messageid'];
					$messageid != "" or die(get_return_code("argument missing") . ": No message id argument **messageread**");
					message_worker($username, $recipient, $messageid, "read");
					exit;
				}
				if (isset($_GET['removemessage'])) {
					$recipient=$_GET['removemessage'];
					$recipient != "" or die(get_return_code("argument missing") . ": No receiver argument **removemessage**");
					$messageid = $_GET['messageid'];
					$messageid != "" or die(get_return_code("argument missing") . ": No message id argument **removemessage**");
					message_worker($username, $recipient, $messageid, "removed");
					exit;
				}

				if (isset($_GET['upload'])) {
					$targetuser = $_GET['targetuser'];
					if ($targetuser)
						$fileDir = $rootdir . $targetuser;
					else
						$fileDir = $rootdir . $username;
					$subdir = $_GET['upload'];
					if ($subdir)
						$fileDir = $fileDir . "/" . $subdir;
					upload_file($fileDir);
					exit;
				}
				if (isset($_GET['file'])) {
					$subdir = isset($_GET['subdir']) ? $_GET['subdir'] . "/" : '';
					$targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] ."/" : $username;
					if ($targetuser == "")
						$targetuser = $username;
					$filedir=$rootdir . $targetuser . $subdir;
					download_file($_GET['file'], $filedir);
					exit;
				}
				if (isset($_GET['getbinfile'])) {
					$binfile = $_GET['getbinfile'];
					if ($binfile) {
						$subdir = isset($_GET['subdir']) ? $_GET['subdir'] . "/" : '';
						$targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] ."/" : $username;
						if ($targetuser == "")
							$targetuser = $username;
						$filedir = $rootdir . $targetuser . $subdir;
						get_binfile($binfile, $filedir);
						exit;
					}
				}
				if (isset($_GET['delfile'])) {
					$subdir = isset($_GET['subdir']) ? $_GET['subdir'] . "/" : '';
					$targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] ."/" : $username;
					if ($targetuser == "")
						$targetuser = $username;
					$file = $rootdir . $username  . "/" . $subdir . $targetuser . $_GET['delfile'];
					if (is_file($file))
						unlink($file);
					exit;
				}

				//?user=1739556367374&password=lrTp1$&getbinfile=resume&fromuser=1739556367374
				if (isset($_GET['checkfilectime'])) {
					$file = $_GET['checkfilectime'];
					if ($file) {
						$targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] : $username;
						if ($targetuser == "")
							$targetuser = $username;
						$subdir = isset($_GET['subdir']) ? $_GET['subdir'] . "/" : '';
						$filename=$rootdir . $targetuser . "/" . $subdir . $file;
						check_file_ctime($filename);
						exit;
					}
				}

				echo get_return_code("custom error") . ": Missing action argument";
			}
			else { //username == admin
				//user management
				$query = isset($_GET['onlineuser']) ? $_GET['onlineuser'] : '';
				if ($query) { //Check if there is an already existing user in the online database. The unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
					$userpassword = isset($_GET['userpassword']) ? $_GET['userpassword'] : '';
					run_dbscript("getid", $query . " '" . $userpassword . "'", "", true);
					exit;
				}

				if (isset($_GET['allusers'])) {
					$users = scan_dir($rootdir, "", true, false, false);
					$new_users = remove_from_string($users, "admin|");
					$new_users = remove_from_string($new_users, "scripts|");
					echo $new_users;
					exit;
				}

				$userid = isset($_GET['onlinedata']) ? $_GET['onlinedata'] : '';
				if ($userid) { //Check if there is an already existing user in the online database. The  unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
					run_dbscript("getall", "", $userid, true);
					exit;
				}

				$username = isset($_GET['checkuser']) ? $_GET['checkuser'] : '';
				if ($username) { //check if user exists
					$user_password = isset($_GET['userpassword']) ? $_GET['userpassword'] : '';
					$return_var = user_exists($username, $user_password);
					echo user_exists_return_message($return_var);
					exit;
				}

				$username = isset($_GET['adduser']) ? $_GET['adduser'] : '';
				if ($username) { //new user creation. Encrypt password onto file and create the user's dir
					$new_user_password = isset($_GET['userpassword']) ? $_GET['userpassword'] : '';
					$return_var = user_exists($username, $new_user_password);
					if ($return_var == 6) {
						$ok = run_htpasswd("-bB", $username, $new_user_password);
						if ($ok == 0) {
							$userdir = $rootdir . $username;
							if (!create_dir($userdir . "/Database")) //Creates the user dir and its database subdir
								$ok = 1;
							if (!create_dir($userdir . "/chats")) //Creates the user dir and its database subdir
								$ok = 1;
						}
						if ($ok == 0)
							echo "0: " . $username . " successfully created";
						else
							echo get_return_code("custom error") . ": Error creating user " . $username;
					}
					else
						echo user_exists_return_message($return_var);
					exit;
				}

				$userid = isset($_GET['alteronlineuser']) ? $_GET['alteronlineuser'] : '';
				if ($userid) { //dbscript expects the file user.data to have been previously uploaded to $userid dir
					if (!isset($_GET['userpassword'])) {
						echo get_return_code("password missing") . ": Cannot update user information. No user password provided.";
						exit;
					}
					$user_password = $_GET['userpassword'];
					$return_var = user_exists($userid, $user_password);
					if ($return_var == 0) {
						if (update_datafile_with_password($userid))
							run_dbscript("add", "", $userid, true);
						else
							echo get_return_code("user does not exist") . ": Cannot update ".$userid." information. User not found on database.";
					}
					else
						echo user_exists_return_message($return_var);
					exit;
				}

				$username = isset($_GET['deluser']) ? $_GET['deluser'] : '';
				if ($username) { //remove user and their dir
					if (!isset($_GET['userpassword'])) {
						echo get_return_code("password missing") . ": Cannot update user information. No user password provided.";
						exit;
					}
					$user_password = $_GET['userpassword'];
					$return_var = user_exists($username, $user_password);
					if ($return_var == 0) {
						del_coach($username);
						$ok = run_htpasswd("-D", $username, "");
						if ($ok == 0) {
							run_dbscript("del", "", $username, false);
							$userdir = $rootdir . $username;
							if (is_dir($userdir)) {
								if (!erasedir($userdir))
									$ok = 1;
							}
						}
						if ($ok == 0)
							echo "0:" . $username . " successfully removed.";
						else
							echo get_return_code("unknown error") . ": " . $username . " not removed.";
					}
					else
						echo user_exists_return_message($return_var);
					exit;
				}

				$username = isset($_GET['changepassword']) ? $_GET['changepassword'] : '';
				if ($username) {
					$cur_password = isset($_GET['oldpassword']) ? $_GET['oldpassword'] : '';
					$return_var = user_exists($username, $cur_password);
					if ($return_var == 0) {
						$new_password = isset($_GET['newpassword']) ? $_GET['newpassword'] : '';
						$ok = run_htpasswd("-bB", $username, $new_password);
						if ($ok == 0)
							echo "0: " . $username . "  password successfully modified";
						else
							echo get_return_code("unknown error") . ": " . $username . " password modification failed";
					}
					else
						echo user_exists_return_message($return_var);
					exit;
				}
			}
		}
		else {
			// Authentication failed
			header('HTTP/1.1 401 Unauthorized');
			echo get_return_code("authentication failed") . ": Authentication Failed. Invalid user or password.";
		}
	}
}
else
	print_r2("Welcome to the TrainingPlanner app server!");
?>
