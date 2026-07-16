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
function verify_credentials($target_user, $password, $htpasswd_file): bool {
	if (!file_exists($htpasswd_file)) {
		die("htpasswd file not found");
	}
	#print_r2("Authenticating " . $target_user . " with password " . $password);
	// Read the .htpasswd file line by line
	$lines = file($htpasswd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	foreach ($lines as $line) {
		// Each line is in format: username:hashed_password
		list($storedUser, $storedHash) = explode(':', $line, 2);

		if ($storedUser === $target_user) {
			// Check password against stored hash
			if (password_verify($password, $storedHash) || crypt($password, $storedHash) === $storedHash) {
				return true;
			}
		}
	}
	return false;
}

function upload_file($uploadDir, $backupDir) {
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
			$uploadFilePath = $uploadDir . '/' . basename($fileName);
			// Move the uploaded file to the upload directory
			if (move_uploaded_file($fileTmpPath, $uploadFilePath)) {
				chper($uploadFilePath);
				if ($backupDir)
					copy($uploadFilePath, $backupDir . '/' . basename($fileName));
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
	if (is_dir($downloadDir)) {
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
				if (connection_aborted()) {
					echo get_return_code("connection aborted") . ": Connection aborted **download_file**";
					return false;
				}
				//only return the contents of the file. Any extra string will only get in the way
				return true;
			}
		}
	}
	echo get_return_code("file not found") . ": $downloadDir/$file not found **download_file**";
	return false;
}

function check_file_ctime($filename) {
	print_r2($filename);
	if (is_file($filename))
		echo "0 :", date('Hisymd', filectime($filename));
	else
		echo get_return_code("file not found") . ": " . $filename . " not found **check_file_ctime**";
}

function scan_dir($path, $pattern, $only_dirs, $only_files, $get_c_time, $recursive) {
	$path = $path . '/';
	$output = "";
	if (is_dir($path)) {
		$files = array_values(array_diff(scandir($path), array('.', '..')));
		if (count($files) > 0) {
			foreach ($files as &$file) {
				if ($recursive && is_dir($path.$file))
					$output .= scan_dir($path.$file . '/', $pattern, $only_dirs, $only_files, $get_c_time, $recursive);

				if (strlen($pattern) > 0) {
					if (!str_contains($file, $pattern))
						continue;
				}
				$skip = $only_files && !is_file($path.$file);
				if ($skip !== true) {
					$skip = $only_dirs && !is_dir($path.$file);
					if ($skip !== true) {
						if ($get_c_time)
							$output .= $file . "|" . date('Hisymd', filectime($path.$file)) . "|";
						else
							$output .= $path.$file."|";
					}
				}
			}
		}
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

function run_commands($target_user, $subdir, $delete_cmdfile) {
	global $rootdir;
	$path = $rootdir.$target_user.'/'.$subdir.'/';
	if (is_dir($path)) {
		$files = array_values(array_diff(scandir($path), array('.', '..')));
		if (count($files) > 0) {
			global $scriptsdir;
			$script=$scriptsdir . "runcmds.sh";
			//ob_start();
			foreach ($files as &$file) {
				if (!str_ends_with($file, ".cmd"))
					continue;
				//passthru("$script $target_user $subdir $file", $return_var);
				//echo "$script $target_user $subdir $file";
				$return_var = shell_exec("$script $target_user $subdir $file");
				if (is_null($return_var)) {
					echo "an error occured or the command produced no output";
				} elseif (!$return_var) {
					echo "the pipe could not be established";
				} else {
					if ($return_var == 0)
						echo "0: " . $file . " executed correctly";
					else
						echo get_return_code("exec failed") . ": " . $file . " (".$return_var.")";
				}
				if ($delete_cmdfile == "1")
					unlink($path.$file);
			}
			//$output = ob_get_clean();
			return true;
		}
	}
	echo get_return_code("directory not found") . ": " . $path . " is empty or does not exist **run_commands**";
	return false;
}

function cmd_downloaded($target_user, $deviceid, $cmd_file) {
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
			$n_downloads++;
			$new_cmds = $new_cmds . "#Downloads " . $n_downloads . "";
		}
		else
			$new_cmds = $new_cmds . $cmd . "";
	}
	$n_devices = get_number_of_devices($target_user);

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

function run_test_function($target_user, $password) {

}

/*require 'php-websocket/vendor/autoload.php';
function initiate_websocket_connection($id, $client_ip, $client_port): bool {
	$client = apcu_fetch($id);
	if ($client === false) {
		$wsUrl = "ws://{$client_ip}:{$client_port}";

		$client = new WebSocket\Client($wsUrl);

		$client
			->setPersistent(true)
			// Add standard middlewares
			->addMiddleware(new WebSocket\Middleware\CloseHandler())
			->addMiddleware(new WebSocket\Middleware\PingResponder())
			// Listen to incoming Text messages
			->onText(function (WebSocket\Client $client, WebSocket\Connection $connection, WebSocket\Message\Text $message) {
				// Act on incoming message
				$words = explode("\036", $message->getContent());
				switch ($words[1]) {
					case "close": $client->close(); break; // Close connection
					case "peer_address": $client->text($words[0]."\036".apcu_fetch($words[2]."ip")); break; //get $words[2](the interlocutor's id) stored ip address
					//default: echo $words[0];
				}
				//foreach ($words as $word) {
				//	echo $word . "\n";
				//}
			})
			// Listen to incoming Binary messages
			->onBinary(function (WebSocket\Client $client, WebSocket\Connection $connection, WebSocket\Message\Binary $message) {
				// Act on incoming message
				echo "Got file: {$message->getContent()} \n";
			})
			->start();
		apcu_store($id, $client);
		apcu_store($id."ip", $client_ip);
	}
	else {
		if (!$client->isConnected()) {
			$client->connect();
			if (!$client->isConnected()) {
				$stored_ip = apcu_fetch($id."ip");
				if ($stored_ip != $client_ip) {
					apcu_delete($id);
					apcu_delete($id."ip");
					return initiate_websocket_connection($id, $client_ip, $client_port);
				}
			}
		}
	}

	return $client->isRunning();

	 //echo "Connected *to client " . $id . "\n";
	 // Send a message
	 //$client->text("*************** Hello $id Server here!");
	 // Read response (this is blocking)
	 //$message = $client->receive();
	 //echo "Got message: {$message->getContent()} \n";
	 // Close connection
	 //$client->close();
}*/

function terminate_websocket_connection($id) {
	$client = apcu_fetch($id);
	if ($client !== false) {
		$client->close();
		apcu_delete($id);
		apcu_delete($id."ip");
	}
}

function set_online_visible($target_user, $visible) {
	global $rootdir;
	$visible_file = $rootdir . $target_user . "/visible";
	if ($visible) {
		$fh = fopen($visible_file, "c");
		$ip = $_SERVER['REMOTE_ADDR'];
		fwrite($fh, $ip . "\n");
		fclose($fh);

		$iter = new APCUIterator(null, APC_ITER_KEY);
		foreach ($iter as $key => $value) {
			echo $value['key'] . "\n";
		}
	}
	else {
		unlink($visible_file);
	}
}

function add_device($target_user, $device_id) {
	global $fileMode;
	global $rootdir;
	$devices_file = $rootdir . $target_user . "/devices.txt";
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

function del_device($target_user, $device_id) {
	global $rootdir;
	$devices_file = $rootdir . $target_user . "/devices.txt";
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

function get_number_of_devices($target_user) {
	global $rootdir;
	$devices_file = $rootdir . $target_user . "/devices.txt";
	$n_devices = 0;
	if (file_exists($devices_file)) {
		$devices = file($devices_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($devices as $device)
			$n_devices++;
	}
	return $n_devices;
}

function get_devices_list($target_user) {
	global $rootdir;
	$devices_file = $rootdir . $target_user . "/devices.txt";
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

function is_device_listed($target_user, $device_id) {
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

function request_coach($target_user, $coach) {
	global $rootdir;
	$requests_file = $rootdir . $coach . "/requests.txt";
	if (!file_exists($requests_file)) {
		$fh = fopen($requests_file, "w") or die("Return code: 10 Unable to create requests file!" .$requests_file . "");
		chper($requests_file);
	}
	else {
		$clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($clients as $line) {
			if ($line == $target_user) {
				echo get_return_code("no changes success") . ": Client's request had already been placed";
				return;
			}
		}
		$fh = fopen($requests_file, "a+") or die(get_return_code("open write failed") . ": Unable to open requests file to append new request " .$requests_file);
	}
	fwrite($fh, $target_user . "\n");
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

function get_tpmessages($owner_user) {
	global $rootdir;
	$tpmessages_file = $rootdir . $owner_user . "/tpmessages.txt";
	if (file_exists($tpmessages_file)) {
		$has_new_messages = apcu_fetch($owner_user."tpmessages");
		if ($has_new_messages === false) {
			echo get_return_code("no new messages") . ": No New TP messages";
			return;
		}
		echo "0: ";
		$messages = file($tpmessages_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($messages as $message)
			echo $message . "\n";
		apcu_store($owner_user."tpmessages", false); //$has_new_messages = false
	}
	else
		echo get_return_code("no messages") . ": No TP messages";
}

function send_tpmessage($target_user, $message) {
	global $rootdir;
	$tpmessages_file = $rootdir . $target_user . "/tpmessages.txt";
	if (!file_exists($tpmessages_file)) {
		$fh = fopen($tpmessages_file, "w") or die(get_return_code("open write failed") . ": Unable to create TP messages file " . $tpmessages_file);
		chper($tpmessages_file);
	}
	else {
		$existing_messages = file($tpmessages_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
		foreach ($existing_messages as $existing_message) {
			if ($existing_message == $message) {
				echo get_return_code("no change success") . ": Message had already been sent";
				return;
			}
		}
		$fh = fopen($tpmessages_file, "a+") or die(get_return_code("open write failed") . ": Unable to append to to TP messages file " . $tpmessages_file);
	}
	fwrite($fh, $message."\n");
	fclose($fh);
	apcu_store($target_user."tpmessages", true); //$has_new_messages = true
	echo "0: TP Message Sent!";
}

function remove_tpmessage($target_user, $message) {
	global $rootdir;
	$tpmessages_file = $rootdir . $target_user . "/tpmessages.txt";
	$tpmessages_arr = file($tpmessages_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	$kept_messages = [];
	$n_removed = 0;
	$n_kept = 0;
	foreach ($messages_arr as $stored_message) {
		if ($stored_message !== $message) {
			$kept_messages[] = $stored_message;
			$n_kept++;
		}
		else
			$n_removed++;
	}
	if ($n_removed > 0) {
		$fh = fopen($tpmessages_file, "w");
		if (count($kept_messages) > 0)
			fwrite($fh, implode($kept_messages));
		else
			fwrite($fh, "");
		fclose($fh);
		echo "0: Message removed: " . $message;
	}
	else
		echo get_return_code("no changes success") . ": Cannot remove message because it was not found " . $message;
}

/**
	record_separator(oct 036, dec 30) separates the message fields
	set_separator (oct 037, dec 31) separates messages of the same sender
	exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and the odd numbers are the sender file names)
*/

function get_newchatmessages($owner_user) {
	global $rootdir;
	$owner_user_dir = $rootdir . $owner_user;
	if (!create_dir($owner_user_dir))
		die(get_return_code("directory not writable") . ": Unable to create dir: " . $owner_user_dir);
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
			$content = $content . file_get_contents($file);
		}
		if ($content != "") {
			echo "0: ";
			echo $content;
			return;
		}
	}
	echo get_return_code("no messages") . ": No new chat messages";
}

function send_chatmessage($sender, $receiver, $message) {
	global $rootdir;
	$messages_dir = $rootdir . $receiver . '/' . $sender . "/chat/";
	if (!create_dir($messages_dir))
		die(get_return_code("directory not writable") . ": Unable to create messages dir " .$messages_dir);
	$messages_file = $messages_dir . "new-messages.chat";

	if (!file_exists($messages_file)) {
		$fh = fopen($messages_file, "w") or die(get_return_code("open write failed") . ": Unable to create messages file " . $messages_file);
		chper($messages_file);
	}
	else
		$fh = fopen($messages_file, "a+") or die(get_return_code("open write failed") . ": Unable to append to to messages file " . $messages_file);
	fwrite($fh, $message . "\037");
	fclose($fh);
	apcu_store("$receiver-$message_file", true); //set $is_modified = true
	echo "0: Message Sent!";
}

function remove_chat_message($sender, $recipient, $message) {
	global $rootdir;
	$messages_dir = $rootdir . $receiver . '/' . $sender . "/chat/";
	$messages_file = $messages_dir . "new-messages.chat";
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

function run_htpasswd($cmd_args, $target_user, $password) {
	global $htpasswd;
	global $htpasswd_file;

	 if (!file_exists($htpasswd_file))
		$cmd_args = $cmd_args . "c";
	$return_var = 0;
	$output = [];
	exec("$htpasswd $cmd_args $htpasswd_file $target_user $password", $output, $return_var);
	return $return_var;
}

function user_exists($target_user, $user_password) {
	return run_htpasswd("-bv", $target_user, $user_password);
}

function user_exists_return_message($return_var) {
	switch ($return_var) {
		case 0: $error_string = "0: User exists and password is correct"; break;
		case 3: $error_string = get_return_code("wrong password") . ": User exists and password is wrong"; break;
		case 6: $error_string = get_return_code("user does not exist") . ": User does not exist"; break;
		default: $error_string = get_return_code("authentication failed") . ": Authentication Failed. Invalid user or password."; break;
	}
	return $error_string;
}

function update_datafile_with_password($target_user) {
	global $rootdir;
	global $htpasswd_file;

	$users = file($htpasswd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	$password="";
	foreach ($users as $user) {
		$sep=strpos($user, ':');
		$target_user=substr($user, 0, $sep);
		if ($target_user == $target_user) {
			$password=substr($user, $sep+1, strlen($user)-$sep);
			break;
		}
	}
	if ($password != "") {
		$userdatafile = $rootdir . $target_user . "/user.data";
		if (file_exists($userdatafile)) {
			$fh = fopen($userdatafile, 'a')  or die(get_return_code("open write failed") . ": Unable to open " .$userdatafile);
			fwrite($fh, $password);
			fclose($fh);
			return true;
		}
	}
	return false;
}

function run_dbscript($cmd, $cmd_opt, $target_user, $print_output) {
	global $scriptsdir;
	$dbscript=$scriptsdir . "usersdb.sh";
	ob_start();
	if ($target_user) {
		if ($cmd_opt)
			passthru("$dbscript $target_user $cmd $cmd_opt", $return_var);
		else
			passthru("$dbscript $target_user $cmd", $return_var);
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

$userid = isset($_GET['user']) ? $_GET['user'] : '';

if ($userid) {
	$password = isset($_GET['password']) ? $_GET['password'] : '';
	if (!$password) {
		die(get_return_code("password missing") . ": Missing password");
	} else {
		if (verify_credentials($userid, $password, $htpasswd_file)) {
			// Authentication successful
			if (isset($_GET['test'])) {
				run_test_function($userid, $password);
				exit;
			}

			if ($userid != "admin") {
				if (isset($_GET['listfiles'])) {
					$subdir = isset($_GET['listfiles']) ? $_GET['listfiles'] . "/" : '';
					$target_user = isset($_GET['fromuser']) ? $_GET['fromuser'] ."/" : $userid;
					if ($target_user == "")
						$target_user = $userid;
					$filedir = $rootdir . $target_user . $subdir;
					$pattern = isset($_GET['pattern']) ? $_GET['pattern'] : '';
					$files = scan_dir($filedir, $pattern, false, true, true, false);
					echo "0: ".$files;
				} elseif (isset($_GET['listdirs'])) {
					$subdir = isset($_GET['listdirs']) ? $_GET['listdirs'] . "/" : '';
					$target_user = isset($_GET['fromuser']) ? $_GET['fromuser'] ."/" : $userid;
					if ($target_user == "")
						$target_user = $userid;
					$filedir = $rootdir . $target_user . $subdir;
					$pattern = isset($_GET['pattern']) ? $_GET['pattern'] : '';
					$files = scan_dir($filedir, $pattern, true, false, false, false);
					echo "0: ".$files;
				} elseif (isset($_GET['runcmds'])) {
					$subdir = $_GET['runcmds'];
					run_commands($userid, $subdir, true);
				} elseif (isset($_GET['downloadcmd'])) {
					$subdir = isset($_GET['subdir']) ? $_GET['subdir'] . "/" : '';
					$filedir = $rootdir . $userid . "/" . $subdir;
					$device_id = $_GET['deviceid'];
					if ($device_id == "")
						die(get_return_code("no device id") . ": Missing device id argument");
					elseif (!is_device_listed($userid, $device_id))
						die(get_return_code("deviceid not registered") . ": Device id not registered");
					if (download_file($_GET['downloadcmd'], $filedir))
						cmd_downloaded($userid, $device_id, $filedir . $_GET['downloadcmd']);
				} elseif (isset($_GET['login'])) {
					$port = $_GET['login'];
					$device_id = $_GET['device'];
					add_device($userid, $device_id);
					set_online_visible($userid, true);
					clear_apcu_cache($userid);
					apcu_store($userid."tpmessages", true); //$has_new_messages = true
					apcu_store($userid, $_SERVER['REMOTE_ADDR'] . ":" .$port);
					echo "0: User $userid logged in";
				} elseif (isset($_GET['logout'])) {
					$device_id = $_GET['device'];
					del_device($userid, $device_id);
					set_online_visible($userid, false);
					clear_apcu_cache($userid);
					echo "0: User $userid logged out";
				} elseif (isset($_GET['getpeeraddress'])) {
					$peer = $_GET['getpeeraddress'];
					$peer_addr = apcu_fetch($peer);
					if ($peer_addr == false)
						echo get_return_code("user does not exist") . ": " . $peer . " is not logged in";
					else
						echo "0: " . $peer_addr;
				} elseif (isset($_GET['getdeviceslist'])) {
					get_devices_list($userid);
				} else if (isset($_GET['changepassword'])) {
					$old_password = $_GET['changepassword'];
					$return_var = user_exists($userid, $old_password);
					if ($return_var === 0) {
						$new_password = $_GET['newpassword'];
						if ($new_password == "")
							die(get_return_code("argument missing") . ": no new passord given **changecredentials**");
						$ok = run_htpasswd("-bB", $userid, $new_password);
						if ($ok == 0)
							echo "0: " . $userid . "  password successfully modified";
						else
							echo get_return_code("unknown error") . ": " . $userid . " password modification failed";
					} else {
						echo user_exists_return_message($return_var);
					}
				} else if (isset($_GET['addcoach'])) {
					add_coach($userid);
				} elseif (isset($_GET['delcoach'])) {
					del_coach($userid);
				} elseif (isset($_GET['getonlinecoaches'])) {
					get_online_coaches();
				} elseif (isset($_GET['requestcoach'])) {
					$coach = $_GET['requestcoach'];
					if ($coach)
						request_coach($userid, $coach);
				} elseif (isset($_GET['deleteclientrequest'])) {
					$client = $_GET['deleteclientrequest'];
					if ($client)
						delete_client_request($userid, $client);
				} elseif (isset($_GET['listclientsrequests'])) {
					list_clients_requests($userid);
				} elseif (isset($_GET['acceptclientrequest'])) {
					$client = $_GET['acceptclientrequest'];
					if ($client)
						accept_client_request($userid, $client);
				} elseif (isset($_GET['rejectclientrequest'])) {
					$client = $_GET['rejecttclientrequest'];
					if ($client)
						reject_client_request($userid, $client);
				} elseif (isset($_GET['deletecoachanswer'])) {
					$coach = $_GET['deletecoachanswer'];
					if ($coach)
						delete_coach_answer($userid, $coach);
				} elseif (isset($_GET['listcoachesanswers'])) {
					list_coaches_answers($userid);
				} elseif (isset($_GET['acceptcoachanswer'])) {
					$coach = $_GET['acceptcoachanswer'];
					if ($coach)
						accept_coach_answer($userid, $coach);
				} elseif (isset($_GET['rejectcoachanswer'])) {
					$coach = $_GET['rejectcoachanswer'];
					if ($coach)
						reject_coach_answer($userid, $coach);
				} elseif (isset($_GET['getclients'])) {
					get_clients_list($userid);
				} elseif (isset($_GET['removecurclient'])) {
					$client = $_GET['removecurclient'];
					if ($client)
						remove_client_from_clients($userid, $client);
				} elseif (isset($_GET['getcoaches'])) {
					get_coaches_list($userid);
				} else if (isset($_GET['removecurcoach'])) {
					$coach = $_GET['removecurcoach'];
					if ($coach)
						remove_coach_from_coaches($userid, $coach);
				} elseif (isset($_GET['gettpmessages'])) {
					get_tpmessages($userid);
				} elseif (isset($_GET['sendtpmessage'])) {
					$receiver = $_GET['sendtpmessage'];
					$message = $_GET['message'];
					$message != "" or die(get_return_code("argument missing") . ": No message argument **sendtpmessage**");
					send_tpmessage($receiver, $message);
				} elseif (isset($_GET['removetpmessage'])) {
					$message = $_GET['removetpmessage'];
					$message != "" or die(get_return_code("argument missing") . ": No message argument **removetpmessage**");
					remove_tpmessage($userid, $message);
				} elseif (isset($_GET['getnewchatmessages'])) {
					get_newchatmessages($userid);
				} elseif (isset($_GET['sendchatmessage'])) {
					$receiver=$_GET['sendchatmessage'];
					$receiver != "" or die(get_return_code("argument missing") . ": No receiver argument **sendchatmessage**");
					$message = $_GET['message'];
					$message != "" or die(get_return_code("argument missing") . ": No message argument **sendchatmessage**");
					send_chatmessage($userid, $receiver, $message);
				} elseif (isset($_GET['removechatmessage'])) {
					$receiver=$_GET['removechatmessage'];
					$receiver != "" or die(get_return_code("argument missing") . ": No receiver argument **removechatmessage**");
					$message = $_GET['message'];
					$message != "" or die(get_return_code("argument missing") . ": No message argument **removechatmessage**");
					remove_chat_message($userid, $receiver, $message);
				} elseif (isset($_GET['forcegetnewmessages'])) {
					clear_apcu_cache($userid);
				}
			} else { //if ($userid != "admin")
				//username == admin
				if (isset($_GET['checkaccount'])) {
					$query = $_GET['checkaccount'];
					//Check if there is an already existing user in the online database.
					//The unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
					$userpassword = isset($_GET['userpassword']) ? $_GET['userpassword'] : '';
					run_dbscript("getid", $query . " '" . $userpassword . "'", "", true);
				} elseif (isset($_GET['allusers'])) {
					$users = scan_dir($rootdir, "", true, false, false, false);
					$new_users = remove_from_string($users, "admin|");
					$new_users = remove_from_string($new_users, "scripts|");
					echo "0: ".$new_users;
				} elseif (isset($_GET['onlinedata'])) {
					//Check if there is an already existing user in the online database. The  unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
					run_dbscript("getall", "", $_GET['onlinedata'], true);
				} elseif (isset($_GET['adduser'])) {
					//new user creation. Encrypt password onto file and create the user's dir
					$user_id = $_GET['adduser'];
					$new_user_password = isset($_GET['userpassword']) ? $_GET['userpassword'] : '';
					$return_var = user_exists($user_id, $new_user_password);
					if ($return_var == 6) {
						$ok = run_htpasswd("-bB", $user_id, $new_user_password);
						if ($ok == 0) {
							$userdir = $rootdir . $user_id;
							if (!create_dir($userdir . "/Database")) //Creates the user dir and its database subdir
								$ok = 1;
							if (!create_dir($userdir . "/chats")) //Creates the user dir and its database subdir
								$ok = 1;
						}
						if ($ok == 0) {
							if (update_datafile_with_password($user_id))
								run_dbscript("add", "", $user_id, true);
							echo "0: " . $user_id . " successfully created";
						}
						else
							echo get_return_code("custom error") . ": Error creating user " . $user_id;
					} else {
						die(user_exists_return_message($return_var));
					}
				} elseif (isset($_GET['deluser'])) {
					//remove user and their dir
					$user_id = $_GET['deluser'];
					if (!isset($_GET['userpassword']))
						die(get_return_code("password missing") . ": Cannot update user information. No user password provided.");
					$user_password = $_GET['userpassword'];
					$return_var = user_exists($user_id, $user_password);
					if ($return_var == 0) {
						del_coach($user_id);
						$ok = run_htpasswd("-D", $user_id, "");
						if ($ok == 0) {
							run_dbscript("del", "", $user_id, false);
							$userdir = $rootdir . $user_id;
							if (is_dir($userdir)) {
								if (!erasedir($userdir))
									$ok = 1;
							}
						}
						if ($ok == 0)
							echo "0:" . $user_id . " successfully removed.";
						else
							echo get_return_code("unknown error") . ": " . $user_id . " not removed.";
					} else {
						//$return_var != 0
						die(user_exists_return_message($return_var));
					}
				}
			} //$userid != "admin"

			/*The following block of functions can be used by either an ordinary user or admin.
			 * Since they are all file operations, they follow the same rationale applied to a TPFilePath:
			 * $owner_user / $target_user / $subdir / $filename
			 * $owner_user is obligatory, as well as $filename.
			**/

			$owner_user = $_GET['owner'];
			if (!$owner_user)
				die(get_return_code("argument missing") . ": Missing *owner* argument");
			$filedir = $rootdir . $owner_user;
			$target_user = $_GET['target'];
			if ($target_user)
				$filedir = $filedir . '/' . $target_user;
			$subdir = $_GET['subdir'];
			if ($subdir)
				$filedir = $filedir . '/' . $subdir;

			if (isset($_GET['upload'])) {
				//When uploading a file to another user, set $targetuser. The sender can only send files to its subdirectory within $targetuser
				if ($userid != "admin") {
					if ($target_user) {
						if ($target_user !== $userid)
							die(get_return_code("no privilege") . ": $userid can only send files to itself or *target* must be set to $userid");
						else //make a copy of the sent file in the user's directory for recovery and/or syncing
							$backupdir = $rootdir . $userid . '/'. $target_user . '/' . $subdir;
					}
				}
				upload_file($filedir, $backupdir);
			} elseif (isset($_GET['checkfilectime'])) {
				check_file_ctime($filedir . '/' . $_GET['checkfilectime']);
			} else {
				//A user can only get/delete files from within their base directory. Admin can get/delete any file
				if ($userid != "admin") {
					if ($owner_user !== $userid)
						die(get_return_code("no privilege") . ": $userid can only get/delete files in its own directory");
				}
				if (isset($_GET['file'])) {
				download_file($_GET['file'], $filedir);
				} elseif (isset($_GET['delfile'])) {
					$file = $filedir . '/' . $_GET['delfile'];
					if (is_file($file))
						unlink($file);
				} else {
					die(get_return_code("argument missing") . ": Missing action argument at: " . __LINE__);
				}
			}
		} else {
			// Authentication failed
			header('HTTP/1.1 401 Unauthorized');
			$return_var = user_exists($target_user, $password);
			echo user_exists_return_message($return_var);
		}
	}
} else {
	echo "Welcome to the TrainingPlanner app server!";
}
?>
