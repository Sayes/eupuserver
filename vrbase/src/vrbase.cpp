#include "IOStreamOutputter.hpp"

#include <xsec/framework/XSECProvider.hpp>
#include <xsec/dsig/DSIGReference.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyHMAC.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xsec/enc/XSECCryptoException.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

XERCES_CPP_NAMESPACE_USE

#ifndef XSEC_NO_XALAN
// Xalan
#include <xalanc/XalanTransformer/XalanTransformer.hpp>
XALAN_USING_XALAN(XalanTransformer)
#endif

#include "vrbase.h"

bool verifireceipt(const std::string& receipt, const std::string& cert) {
    bool result = false;

    try {
        XMLPlatformUtils::Initialize();
        XSECPlatformUtils::Initialise();
    } catch (const XMLException& e) {
    }

    // Use xerces to parse the document
    XercesDOMParser* parser = new XercesDOMParser;
    parser->setDoNamespaces(true);
    parser->setCreateEntityReferenceNodes(true);
    parser->setDoSchema(true);

    // Create an input source
    MemBufInputSource* memIS =
        new MemBufInputSource((const XMLByte*)receipt.c_str(),
                              (unsigned int)receipt.length(), "XSECMem");

    do {
        xsecsize_t errorCount = 0;
        parser->parse(*memIS);
        errorCount = parser->getErrorCount();
        if (errorCount > 0) break;

        DOMDocument* doc = parser->getDocument();
        docSetup(doc);

        // Now create a signature object to validate the document
        XSECProvider prov;
        DSIGSignature* sig = prov.newSignatureFromDOM(doc);

        try {
            // Use the OpenSSL interface objects to get a signing key
            OpenSSLCryptoX509* x509 = new OpenSSLCryptoX509();
            x509->loadX509Base64Bin(cert.c_str(), (unsigned int)cert.length());
            sig->load();
            sig->setSigningKey(x509->clonePublicKey());

            if (sig->verify()) {
                result = true;
            }
            delete x509;
        } catch (XSECException& e) {
            break;
        } catch (XSECCryptoException& e) {
            break;
        }  // END try

    } while (0);

    delete memIS;
    delete parser;

    return result;
}
