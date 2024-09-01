// from: https://github.com/wkh237/react-native-fetch-blob/blob/master/android/src/main/java/com/RNFetchBlob/Utils/PathResolver.java
// MIT License, see: https://github.com/wkh237/react-native-fetch-blob/blob/master/LICENSE
// original copyright: Copyright (c) 2017 xeiyan@gmail.com
// src slightly modified to be used into Qt Projects: (c) 2017 ekke@ekkes-corner.org

package org.vivenciasoftware.TrainingPlanner;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.content.ContentUris;
import android.os.Environment;
import android.content.ContentResolver;
import android.util.Log;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.lang.NumberFormatException;

public class QSharePathResolver
{
	public final static String TAG = "************** QSharePathResolver ***************	  ";

    public static String getRealPathFromURI(final Context context, final Uri uri) {
		Log.d("************** QSharePathResolver::getRealPathFromURI ***************	  ", uri.getAuthority());
		if (DocumentsContract.isDocumentUri(context, uri)) { // DocumentProvider
			if (isExternalStorageDocument(uri)) { // ExternalStorageProvider
				Log.d(TAG," isExternalStorageDocument");
				final String docId = DocumentsContract.getDocumentId(uri);
				final String[] split = docId.split(":");
				final String type = split[0];

				if ("primary".equalsIgnoreCase(type))
					return Environment.getExternalStorageDirectory() + "/" + split[1];
				// TODO handle non-primary volumes
			}
			else if (isDownloadsDocument(uri)) { // DownloadsProvider
				Log.d(TAG," isDownloadsDocument");
				final String id = DocumentsContract.getDocumentId(uri);
				Log.d(TAG," getDocumentId "+id);
				long longId = 0;
				try
				{
					longId = Long.valueOf(id);
				}
				catch(NumberFormatException nfe)
				{
					return getDataColumn(context, uri, null, null);
				}
				final Uri contentUri = ContentUris.withAppendedId(Uri.parse("content://downloads/public_downloads"), longId);
				return getDataColumn(context, contentUri, null, null);
			}
			else if (isMediaDocument(uri)) { // MediaProvider
				Log.d(TAG," isMediaDocument");
				final String docId = DocumentsContract.getDocumentId(uri);
				final String[] split = docId.split(":");
				final String type = split[0];

				Uri contentUri = null;
				if ("image".equals(type))
					contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
				else if ("video".equals(type))
					contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
				else if ("audio".equals(type))
					contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;

				final String selection = "_id=?";
				final String[] selectionArgs = new String[] { split[1] };
				return getDataColumn(context, contentUri, selection, selectionArgs);
			}
			else if ("content".equalsIgnoreCase(uri.getScheme())) {
				Log.d(TAG," is uri.getScheme()");
				// Return the remote address
				if (isGooglePhotosUri(uri))
					return uri.getLastPathSegment();
				return getDataColumn(context, uri, null, null);
			}
			else { // Other Providers
				Log.d("************** QSharePathResolver::getRealPathFromURI ***************	  ","is Other Provider");
				try {
					InputStream attachment = context.getContentResolver().openInputStream(uri);
					if (attachment != null) {
						String filename = getContentName(context.getContentResolver(), uri);
						if (filename != null) {
							File file = new File(context.getCacheDir(), filename);
							FileOutputStream tmp = new FileOutputStream(file);
							byte[] buffer = new byte[1024];
							while (attachment.read(buffer) > 0) {
								tmp.write(buffer);
							}
							tmp.close();
							attachment.close();
							return file.getAbsolutePath();
						}
					}
				} catch (Exception e) {
					// TODO SIGNAL shareError()
					return null;
				}
			}
		}
		else if ("content".equalsIgnoreCase(uri.getScheme())) { // MediaStore (and general)
			Log.d("************** QSharePathResolver::getRealPathFromURI ***************	  ","NOT DocumentsContract.isDocumentUri");
			Log.d(TAG," is uri.getScheme()");
			// Return the remote address
			if (isGooglePhotosUri(uri))
				return uri.getLastPathSegment();
			Log.d(TAG," return: getDataColumn ");
			return getDataColumn(context, uri, null, null);
		}
		else if ("file".equalsIgnoreCase(uri.getScheme())) { // File
			Log.d("************** QSharePathResolver::getRealPathFromURI ***************	  ","NOT DocumentsContract.isDocumentUri");
			Log.d(TAG," is file scheme");
			return uri.getPath();
		}
		return null;
    } //getRealPathFromURI

	private static String getContentName(ContentResolver resolver, Uri uri) {
		Cursor cursor = resolver.query(uri, null, null, null, null);
		cursor.moveToFirst();
		int nameIndex = cursor.getColumnIndex(MediaStore.MediaColumns.DISPLAY_NAME);
		if (nameIndex >= 0) {
			String name = cursor.getString(nameIndex);
			cursor.close();
			return name;
		}
		cursor.close();
		return null;
	}

    /**
     * Get the value of the data column for this Uri. This is useful for
     * MediaStore Uris, and other file-based ContentProviders.
     *
     * @param context The context.
     * @param uri The Uri to query.
     * @param selection (Optional) Filter used in the query.
     * @param selectionArgs (Optional) Selection arguments used in the query.
     * @return The value of the _data column, which is typically a file path.
     */
    public static String getDataColumn(Context context, Uri uri, String selection,
                                       String[] selectionArgs) {

        Cursor cursor = null;
        String result = null;
        final String column = "_data";
        final String[] projection = {
                column
        };

        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                final int index = cursor.getColumnIndexOrThrow(column);
                result = cursor.getString(index);
            }
        }
        catch (Exception ex) {
            ex.printStackTrace();
            return null;
        }
        finally {
            if (cursor != null)
                cursor.close();
        }
        return result;
    }


    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    public static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is Google Photos.
     */
    public static boolean isGooglePhotosUri(Uri uri) {
        return "com.google.android.apps.photos.content".equals(uri.getAuthority());
    }

}
