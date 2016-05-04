package
{
	import flash.utils.ByteArray;

	[Embed(source="${path}", mimeType="${mime}")]
	public class ${name} extends ByteArray {
		public function ${name} () {
			super ();
		}
	}
}