package
{
	import flash.display.BitmapData;

	[Embed(source="${path}", mimeType="${mime}")]
	public class ${name} extends BitmapData {
		public function ${name} (width:int, height:int, transparent:Boolean = true, fillColor:uint = 0xFFFFFFFF) {
			super (width, height, transparent, fillColor);
		}
	}
}