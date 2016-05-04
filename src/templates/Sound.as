package
{
	import flash.display.BitmapData;
	import flash.media.Sound;
	import flash.media.SoundLoaderContext;
	import flash.net.URLRequest;

	[Embed(source="${path}", mimeType="${mime}")]
	public class ${name} extends Sound {
		public function ${name} (stream:URLRequest = null, context:SoundLoaderContext = null) {
			super (stream, context);
		}
	}
}