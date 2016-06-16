#include <maya/MPxFileTranslator.h>

namespace ion
{
	namespace toolchain
	{
		class IonMeshExporter : public MPxFileTranslator
		{
		public:

			static void* creator();

			IonMeshExporter();
			virtual ~IonMeshExporter();
			virtual MStatus writer(const MFileObject& filename, const MString& optionsString, FileAccessMode mode);
			virtual bool haveReadMethod() const;
			virtual bool haveWriteMethod() const;
			virtual bool haveNamespaceSupport() const;
			virtual bool haveReferenceMethod() const;
			virtual bool allowMultipleFileOptimization() const;

			virtual MString defaultExtension() const;
			virtual bool canBeOpened() const;
			virtual MPxFileTranslator::MFileKind identifyFile(const MFileObject& filename, const char* buffer, short size) const;
		};
	}
}
